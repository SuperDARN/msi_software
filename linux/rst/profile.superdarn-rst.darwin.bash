# profile-darwin.bash
# ===================

export OSTYPE="darwin"
export SYSTEM="darwin"
export EDITOR="emacs -nw"

#####################################################################
#                                                                   #
# Set RSTPATH Directory if missing                                  #
#                                                                   #
#####################################################################

if [ -z "${RSTPATH}" ]
then
  export RSTPATH="/Applications/rst"
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

export WWWPATH="/www/doc"
export URLBASE="/doc"

export LIBPATH="${RSTPATH}/lib"
export BINPATH="${RSTPATH}/bin"
export IPATH="${RSTPATH}/include"

export USR_CODEBASE="${RSTPATH}/usr/codebase"
export USR_LIBPATH="${RSTPATH}/usr/lib"
export USR_BINPATH="${RSTPATH}/usr/bin"
export USR_IPATH="${RSTPATH}/usr/include"

export PATH="${PATH}:/opt/local/bin:${BUILD}/bin:${BUILD}/script:${RSTPATH}/bin:${RSTPATH}/usr/bin:${RSTPATH}/script:${HOME}/bin:${HOME}/script:/Applications/itt/idl/bin:${CDF_PATH}/bin"

#####################################################################
#                                                                   #
# Location of pnmtopng                                              #
#                                                                   #
#####################################################################

export PNMTOPNG="/opt/local/bin/pnmtopng"

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

export NETCDF_PATH="/opt/local/"

# pathname for the CDF software

export CDF_PATH="/Applications/cdf"

# IDL header directory

export IDL_IPATH="/Applications/itt/idl/external/include"

#####################################################################
#                                                                   #
# Library directories                                               #
#                                                                   #
#####################################################################

export DYLD_LIBRARY_PATH=.:/Applications/cdf/lib

#####################################################################
#                                                                   #
# IDL Configuration                                                 #
#                                                                   #
#####################################################################


export IDL_PATH="+/${RSTPATH}/idl/lib:+/Applications/itt"
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

#####################################################################
#                                                                   #
# SuperDARN ROS Environment                                         #
#                                                                   #
#####################################################################

export SD_RAW_PATH="/data/ros/rawacf"
export SD_FIT_PATH="/data/ros/fitacf"
export SD_IQ_PATH="/data/ros/iqdat"
export SD_ERRLOG_PATH="/data/ros/errlog";



