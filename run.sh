pdir=$(pwd)
make || exit
exec $pdir/processor/sniffy.out &
sleep 1
exec $pdir/preprocessor/target/debug/sniffy &

while :; do
    :
done

trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

