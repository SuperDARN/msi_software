# profile.linux.bash
# ==================

export OSTYPE="linux"
export SYSTEM="linux"
export EDITOR="emacs -nw"


#####################################################################
#                                                                   #
# Set RSTPATH Directory if missing                                  #
#                                                                   #
#####################################################################

if [ -z "${RSTPATH}" ]
then
  export RSTPATH="${HOME}/rst"
fi

#####################################################################
#                                                                   #
# Directory Paths                                                   #
#                                                                   #
#####################################################################

export BUILD="${RSTPATH}/build"
export CODEBASE="${RSTPATH}/codebase"
export LOGPATH="${RSTPATH}/log"
export DOCPATH="${RSTPATH}/doc"

export WWWPATH="${DOCPATH}/www"
export URLBASE="/doc"

export LIBPATH="${RSTPATH}/lib"
export BINPATH="${RSTPATH}/bin"
export IPATH="${RSTPATH}/include"

export USR_CODEBASE="${RSTPATH}/usr/codebase"
export USR_LIBPATH="${RSTPATH}/usr/lib"
export USR_BINPATH="${RSTPATH}/usr/bin"
export USR_IPATH="${RSTPATH}/usr/include"

export PATH="${PATH}:${BUILD}/bin:${BUILD}/script:${RSTPATH}/bin:${RSTPATH}/usr/bin:${RSTPATH}/script"

export LD_LIBRARY_PATH="${LIBPATH}:${USR_LIBPATH}"

#####################################################################
#                                                                   #
# Location of pnmtopng                                              #
#                                                                   #
#####################################################################

export PNMTOPNG="/usr/bin/pnmtopng"

#####################################################################
#                                                                   #
# Makefiles                                                         #
#                                                                   #
#####################################################################

export MAKECFG=${BUILD}/make/makecfg
export MAKEBIN=${BUILD}/make/makebin
export MAKELIB=${BUILD}/make/makelib

#####################################################################
#                                                                   #
# Compilation directives                                            #
#                                                                   #
#####################################################################

# Libraries for TCPIP

export TCPIPLIBS=" "

# Path of the X11 packages

export XPATH="/usr/X11R6"

# Compile netCDF software

export NETCDF_PATH="/usr/local/netcdf"

# pathname for the CDF software

export CDF_PATH="/usr/local/cdf"

# IDL header directory

export IDL_IPATH="/usr/local/itt/idl/external/include"

#####################################################################
#                                                                   #
# IDL Configuration                                                 #
#                                                                   #
#####################################################################

export IDL_PATH="+/usr/local/itt:+/${RSTPATH}/idl/lib"
export IDL_STARTUP="${RSTPATH}/idl/startup.pro"

#####################################################################
#                                                                   #
# Font Data Tables                                                  #
#                                                                   #
#####################################################################

export FONTPATH=${RSTPATH}"/tables/base/fonts"
export FONTDB=${RSTPATH}"/tables/base/fonts/fontdb.xml"

#####################################################################
#                                                                   #
# Coastline tables                                                  #
#                                                                   #
#####################################################################

export MAPDATA=${RSTPATH}"/tables/general/map_data"
export BNDDATA=${RSTPATH}"/tables/general/bnd_data"

#####################################################################
#                                                                   #
# SuperDARN Environment                                             #
#                                                                   #
#####################################################################

export ISTP_PATH="/data/istp"

export SD_HDWPATH="${RSTPATH}/tables/superdarn/hdw/"
export SD_RADAR="${RSTPATH}/tables/superdarn/radar.dat"

export AACGM_DAT_PREFIX=${RSTPATH}"/tables/analysis/aacgm/aacgm_coeffs"
export IGRF_PATH=${RSTPATH}"/tables/analysis/mag/"
export SD_MODEL_TABLE=${RSTPATH}"/tables/superdarn/model"

export AACGM_v2_DAT_PREFIX="${RSTPATH}/tables/analysis/aacgm/aacgm_coeffs-12-"
export IGRF_COEFFS="${IGRF_PATH}magmodel_1590-2015.txt"

export COLOR_TABLE_PATH=${RSTPATH}"/tables/base/key/"

#####################################################################
#                                                                   #
# SuperDARN ROS Environment                                         #
#                                                                   #
#####################################################################

export SD_RAW_PATH="/data/ros/rawacf"
export SD_FIT_PATH="/data/ros/fitacf"
export SD_IQ_PATH="/data/ros/iqdat"
export SD_ERRLOG_PATH="/data/ros/errlog";
