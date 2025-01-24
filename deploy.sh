#!/usr/bin/env sh

sip="10.49.136.143"

echo "Attempting to ping server ($sip)"
if ! ping -c 1 $sip > /dev/null 2>&1 ; then
    echo "Server at IP $sip not available"
    exit
fi

if ! ssh $sip "git clone https://github.com/BradyBangasser/sniffy && cd sniffy && ./deploy_client.sh"; then
    echo "Failed"
    exit
fi
