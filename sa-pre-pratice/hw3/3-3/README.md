# About

This project used below files:

- `/usr/local/etc/rc.d/ftp-watchd`
- `/etc/rc.conf`
- `/usr/local/bin/ftp_watcher.sh`


## /etc/rc.conf
```sh
pureftpd_enable="YES"
ftp_watchd_enable="YES"
ftp_watchd_command="/usr/local/bin/ftp_watcher.sh"
```

## /usr/local/bin/ftp_watcher.sh
```sh
#!/bin/sh
echo "$(date): $UPLOAD_VUSER has upload file $1 witch size $UPLOAD_SIZE" >> /var/log/uploadscript.log
```
