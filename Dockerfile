# Dockerfile
#
#
# @author      Kealan McCusker <kealan.mccusker@miracl.com>
# -----------------------------------------------------------------------------

FROM scratch
ADD main /
CMD ["/main &"]
