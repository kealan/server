# Dockerfile
#
#
# @author      Kealan McCusker <kealan.mccusker@miracl.com>
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# NOTES:
#
# This script requires Docker (https://www.docker.com/)
#
#
# To create the container execute:
#     docker build --tag="scratch:latest" .
#
# To log into the newly created container:
#     docker run -it -p 8001:8000 scratch
#
# To test
#     curl -d '{"key1":"value1", "key2":"value2"}' -H "Content-Type: application/json" -X POST http://127.0.0.1:8001/data
#
# To stop
# docker stop [container id]
# -----------------------------------------------------------------------------


FROM scratch
ADD main /
CMD ["/main"]
