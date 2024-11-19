#!/usr/bin/env sh

sip="10.49.136.143"

echo "Attempting to ping server ($sip)"
if ! ping -c 4 $sip > /dev/null 2>&1 ; then
    echo "Server at IP $sip not available"
    exit
fi

# ensure 
if ! ssh $sip; then
    echo "Failed to access $sip with ssh, ensure you have the correct address"
    exit
fi

# g

python3 go-builder.py
# scp -r main.py api ds:
