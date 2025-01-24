pdir=$(pwd)
if ! make -C $pdir/processor; then
    echo "Failed to build processor"
    exit
fi

if ! cargo build --manifest-path $pdir/preprocessor/Cargo.toml -r; then
    echo "Failed to build preprocessor"
    exit
fi

exec $pdir/processor/sniffy.out &
sleep 1
exec $pdir/preprocessor/target/release/sniffy &

while :; do
    :
done

trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

