#!/bin/sh
#
# test.sh
#
# Test the server
#
# @author Kealan McCusker <kealanmccusker@gmail.com>>
# -----------------------------------------------------------------------------

# NOTES:

cd ./test

# Change tag to latest version
cp docker-compose.yml.tpl docker-compose.yml
sed -i "s/tag/$VERSION/" docker-compose.yml

# Start service and test
docker-compose up --build &
sleep 240
docker-compose  logs request > response.txt
more response.txt
docker-compose down

# Check logs
grep "HTTP/1.1 200 OK" ./response.txt
if [ $? = 0 ]; then
    echo "TEST1 PASSED"
else 
    echo "TEST1 FAILED"
    exit 1
fi

grep "HTTP/1.1 200 OK" ./response.txt
if [ $? = 0 ]; then
    echo "TEST2 PASSED"
else 
    echo "TEST2 FAILED"
    exit 1
fi

exit 0

