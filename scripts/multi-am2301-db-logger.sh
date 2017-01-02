#!/bin/bash

function log_sensor_data {

    LINE=`grep $1_temp_1m /proc/multi-am2301`;
    if [ $? -ne 0 ]; then
        echo "Problem with grep $1_temp_1m";
        exit $?;
    fi
    COLS=( $LINE );
    T=${COLS[2]};

    LINE=`grep $1_RH /proc/multi-am2301`;
    if [ $? -ne 0 ]; then
        echo "Problem with grep $1_RH";
        exit $?;
    fi

    COLS=( $LINE );
    RH=${COLS[2]};

    LINE=`grep $1_timestamp /proc/multi-am2301`;
    if [ $? -ne 0 ]; then
        echo "Problem with grep $1_timestamp";
        exit $?;
    fi
    COLS=( $LINE );
    TIMESTAMP=${COLS[2]};

    DATETIME=`date --date @${TIMESTAMP} +'%Y-%m-%d %H:%M:%S'`;

    sqlite3 /var/local/am2301-db.sl3 "INSERT INTO $2 VALUES('$DATETIME', '$T', '$RH');";
}


     pins=(    23            24      25      8      7 )
db_tables=( pin23   root_cellar   pin25   pin8   pin7 )

for a in {0..4}
do
  log_sensor_data ${pins[$a]} ${db_tables[$a]}
done 
