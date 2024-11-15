package routes

import (
	api_routes_people "api/routes/people"
	api_routes_people___pid__ "api/routes/people/__pid__"
	api_routes_auth "api/routes/auth"
	api_routes_inmates "api/routes/inmates"
	api_routes_inmates___pid__ "api/routes/inmates/__pid__"
	api_routes_test "api/routes/test"
	"github.com/gin-gonic/gin"
)

func CreateRouter(r *gin.Engine) {
	r_people := r.Group("/people"); {
		r_people.Use(api_routes_people.MIDDLEWARE)
		r_people___pid__ := r_people.Group("/:pid"); {
			r_people___pid__.PUT("/", api_routes_people___pid__.PUT)
			r_people___pid__.GET("/", api_routes_people___pid__.GET)
		}
		r_people.GET("/", api_routes_people.GET)
	}
	r_auth := r.Group("/auth"); {
		r_auth.GET("/", api_routes_auth.GET)
	}
	r_inmates := r.Group("/inmates"); {
		r_inmates.Use(api_routes_inmates.MIDDLEWARE)
		r_inmates___pid__ := r_inmates.Group("/:pid"); {
			r_inmates___pid__.PUT("/", api_routes_inmates___pid__.PUT)
			r_inmates___pid__.GET("/", api_routes_inmates___pid__.GET)
		}
		r_inmates.GET("/", api_routes_inmates.GET)
	}
	r_test := r.Group("/test"); {
		r_test.POST("/", api_routes_test.POST)
	}
	r.GET("/", GET)
}
