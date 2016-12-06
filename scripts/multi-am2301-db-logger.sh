#!/bin/bash

function log_sensor_data {
    LINE=`grep $1_temperature /proc/multi-am2301`;
    if [ $? -ne 0 ]; then
        echo "Problem with grep $1_temperature";
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

log_sensor_data 23 pin23
log_sensor_data 24 root_cellar
log_sensor_data 25 pin25
log_sensor_data 8 pin8
log_sensor_data 7 pin7
