shopt -s extglob
rm -vr !(*.sh)
/bin/bash build.sh
