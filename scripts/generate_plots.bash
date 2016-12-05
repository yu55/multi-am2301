#!/bin/bash

for file in  /var/www/html/*.plt
do
  gnuplot "$file"
done
