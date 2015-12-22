#!/bin/bash

LINE=`grep 23_temperature /proc/multi-am2301`;
if [ $? -ne 0 ]; then
    echo "Problem with grep Temperature";
    exit $?;
fi
COLS=( $LINE );
T=${COLS[2]};

LINE=`grep 23_RH /proc/multi-am2301`;
if [ $? -ne 0 ]; then
    echo "Problem with grep RH";
    exit $?;
fi

COLS=( $LINE );
RH=${COLS[2]};

LINE=`grep 23_timestamp /proc/multi-am2301`;
if [ $? -ne 0 ]; then
    echo "Problem with grep Timestamp";
    exit $?;
fi
COLS=( $LINE );
TIMESTAMP=${COLS[2]};

DATETIME=`date --date @${TIMESTAMP} +'%Y-%m-%d %H:%M:%S'`;

sqlite3 /home/pi/database.sl3 "INSERT INTO pin23 VALUES('$DATETIME', '$T', '$RH');";



LINE=`grep 24_temperature /proc/multi-am2301`;
if [ $? -ne 0 ]; then
    echo "Problem with grep Temperature";
    exit $?;
fi
COLS=( $LINE );
T=${COLS[2]};

LINE=`grep 24_RH /proc/multi-am2301`;
if [ $? -ne 0 ]; then
    echo "Problem with grep RH";
    exit $?;
fi

COLS=( $LINE );
RH=${COLS[2]};

LINE=`grep 24_timestamp /proc/multi-am2301`;
if [ $? -ne 0 ]; then
    echo "Problem with grep Timestamp";
    exit $?;
fi
COLS=( $LINE );
TIMESTAMP=${COLS[2]};

DATETIME=`date --date @${TIMESTAMP} +'%Y-%m-%d %H:%M:%S'`;

sqlite3 /home/pi/database.sl3 "INSERT INTO pin24 VALUES('$DATETIME', '$T', '$RH');";



LINE=`grep 25_temperature /proc/multi-am2301`;
if [ $? -ne 0 ]; then
    echo "Problem with grep Temperature";
    exit $?;
fi
COLS=( $LINE );
T=${COLS[2]};

LINE=`grep 25_RH /proc/multi-am2301`;
if [ $? -ne 0 ]; then
    echo "Problem with grep RH";
    exit $?;
fi

COLS=( $LINE );
RH=${COLS[2]};

LINE=`grep 25_timestamp /proc/multi-am2301`;
if [ $? -ne 0 ]; then
    echo "Problem with grep Timestamp";
    exit $?;
fi
COLS=( $LINE );
TIMESTAMP=${COLS[2]};

DATETIME=`date --date @${TIMESTAMP} +'%Y-%m-%d %H:%M:%S'`;

sqlite3 /home/pi/database.sl3 "INSERT INTO pin25 VALUES('$DATETIME', '$T', '$RH');";


