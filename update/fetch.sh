#!/bin/bash

#backup last files
for f in *.zip *.gz; do
    if [[ ! `echo $f | grep old` ]]; then
        mv $f old-$f
    fi;
done;
# download main sql files
for t in `cat names`; do
    wget http://lib.rus.ec/sql/lib.$t.sql.gz;
done;
# warning: these table links to users table!
# don't download if you are making a mirror site!
wget http://lib11.rus.ec/sql/librate.sql.gz  
wget http://lib11.rus.ec/sql/liblog.sql.gz
# sources
wget http://lib.rus.ec/sql/librusec.zip
