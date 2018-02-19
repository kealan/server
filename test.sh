#!/bin/sh
#
# dockerbuild.sh
#
# Test the server
#
# @author Kealan McCusker <kealanmccusker@gmail.com>>
# -----------------------------------------------------------------------------

# NOTES:
# This script requires Docker

exit 0;

echo "start service"
docker run -it -p 8001:8000 $DOCKER_USERNAME/server-demo 

echo "make client request"
sleep 200
curl -H "Content-Type: application/json" http://localhost:8001/status  --trace-ascii /dev/stdout

# Stop amd remove all containers;
docker stop $(docker ps -a -q)
docker rm $(docker ps -a -q)
