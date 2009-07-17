#customize your database login and port here
DBNAME='librusec'
USER='librusec'
ASK_PASS='-p' # comment out if shouldn't use password

if type -P gzcat &>/dev/null; then
    gzcat *.sql.gz | mysql -u $USER $DBNAME $ASK_PASS # for Mac
else
    zcat *.sql.gz | mysql -u $USER $DBNAME $ASK_PASS # for Linux
fi
