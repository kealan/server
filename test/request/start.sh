#!/bin/sh
#
# start.sh
#
# Test the server
#
# @author Kealan McCusker <kealanmccusker@gmail.com>>
# -----------------------------------------------------------------------------

# NOTES:

echo "make client request"
sleep 200
/usr/bin/curl -H "Content-Type: application/json" http://server-demo:8000/status  --trace-ascii /dev/stdout 
echo "make client finished"
sleep 500

