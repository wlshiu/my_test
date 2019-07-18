#!/bin/bash
set -e 

tail -n +8 dhcpd.leases | less | awk 'BEGIN{RS=ORS="}"}{print $2,$21,"\n"}'| sed 's/}//g' | sort -n