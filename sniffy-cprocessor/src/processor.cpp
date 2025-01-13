#include <rapidjson/writer.h>
#include <mysql/mysql.h>

#include "arrest.hpp"
#include "roster.h"
#include "logging.h"
#include "database.h"
#include "processor.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wwritable-strings"
static constexpr char insert_stmt[] = "INSERT INTO roster (PID, AID) VALUES (?, ?)";
static constexpr char delete_stmt[] = "DELETE FROM roster WHERE PID=?";
#pragma clang diagnostic pop

void Processor::process_json_string(const char *str) {
    Processor::process_json_string(rapidjson::StringStream(str));
}

void Processor::process_json_string(rapidjson::StringStream str) {
    uint32_t total_processed = 0, inserted = 0, updated = 0, deleted = 0, i;
    static uint64_t id_len = 32;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];

    rapidjson::Document doc;

    struct Roster *roster = fetch_roster(database::get_connection());

    stmt = mysql_stmt_init(database::get_connection());
    if (stmt == NULL) {
        ERROR("Failed to initialize stmt, out of memory\n");
        roster_free(roster);
        return;
    }

    if (mysql_stmt_prepare(stmt, insert_stmt, sizeof(insert_stmt) / sizeof(char))) {
        ERRORF("Failed to prepare stmt, error: %s\n", mysql_stmt_error(stmt));
        roster_free(roster);
        mysql_stmt_close(stmt);
        return;
    }

    doc.ParseStream(str);
    if (doc.IsNull() || (!doc.IsObject() && !doc.IsArray())) {
        return;
    }

    rapidjson::GenericArray inmates = doc.IsObject() ? doc.GetObject().FindMember("inmates")->value.GetArray() : doc.GetArray();

    for (auto &inmate : inmates) {
        Arrest a = Arrest::from_json(inmate.GetObject());

        struct RosterEntry *ent = roster_remove(roster, a.get_person()->get_id());

        if (ent != NULL) {
            bool change = false;
            Arrest *ex_arr = Arrest::fetch(ent->aid, database::get_connection());
            a.set_id(ex_arr->get_id());
            std::vector<Charge> &roster_charges = a.get_charges();
            std::vector<Charge> &existing_charges = ex_arr->get_charges(); // becomes removed charges
            std::vector<Charge> new_charge_vec;
            
            for (Charge &roster_charge : roster_charges) {
                std::vector<Charge>::iterator iter;
                for (iter = existing_charges.begin(); iter != existing_charges.end(); iter++) {
                    if ((iter->get_id() >> 37) == (roster_charge.get_id() >> 37)) {
                        roster_charge.set_id(iter->get_id());
                        if (roster_charge.get_bond() != iter->get_bond()) {
                            change = true;
                        }

                        break;
                    }
                }

                if (iter == existing_charges.end()) {
                    change = true;
                    roster_charge.add_note("Filed after initial charges");
                } else {
                    existing_charges.erase(iter, iter + 1);
                }

                new_charge_vec.push_back(roster_charge);
            }

            if (change) {
                ex_arr->swap_charges(new_charge_vec);
                ex_arr->upsert(database::get_connection());
                updated++;
            }

            roster_free_entry(ent);
        } else {
            if (!a.upsert(database::get_connection())) {
                ERROR("Failed to upsert data\n");
            }

            memset(bind, 0, sizeof(bind));

            bind[0].buffer_type = MYSQL_TYPE_BLOB;
            bind[0].length = &id_len;
            bind[0].buffer_length = 32;
            bind[0].buffer = const_cast<uint8_t *>(a.get_person()->get_id());

            uint64_t aid = a.get_id();
            bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
            bind[1].buffer = &aid;

            if (mysql_stmt_bind_param(stmt, bind)) {
                ERRORF("Failed to bind stmt, error: %s\n", mysql_stmt_error(stmt));
                roster_free(roster);
                mysql_stmt_close(stmt);
                return;
            }

            if (mysql_stmt_execute(stmt)) {
                ERRORF("Failed to execute stmt, error: %s\n", mysql_stmt_error(stmt));
                roster_free(roster);
                mysql_stmt_close(stmt);
                return;
            }

            inserted++;
        }

        total_processed++;
    }

    if (mysql_stmt_prepare(stmt, delete_stmt, sizeof(delete_stmt) / sizeof(char))) {
        ERRORF("Failed to prepare stmt, error: %s\n", mysql_stmt_error(stmt));
        roster_free(roster);
        mysql_stmt_close(stmt);
        return;
    }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_BLOB;
    bind[0].buffer_length = id_len;
    bind[0].length = &id_len;

    for (i = 0; i < roster->size; i++) {
        if ((uint64_t) roster->pool[i] > 1) {
            bind[0].buffer = const_cast<uint8_t *>(roster->pool[i]->pid);
            if (mysql_stmt_bind_param(stmt, bind)) {
                ERRORF("Failed to bind stmt, error: %s\n", mysql_stmt_error(stmt));
                roster_free(roster);
                mysql_stmt_close(stmt);
                return;
            }

            if (mysql_stmt_execute(stmt)) {
                ERRORF("Failed to execute stmt, error: %s\n", mysql_stmt_error(stmt));
                roster_free(roster);
                mysql_stmt_close(stmt);
                return;
            }

            deleted++;
        }
    }

    mysql_stmt_close(stmt);

    roster_free(roster);

    SUCCESSF("Successfully processed %d inmates, %d were inserted into the roster, %d were removed, and %d were updated\n", total_processed, inserted, deleted, updated);
}
