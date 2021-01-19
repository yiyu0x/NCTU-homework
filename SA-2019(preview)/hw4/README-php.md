# HW4

ref: [https://www.cyberciti.biz/faq/freebsd-install-php-7-2-with-fpm-for-nginx/](https://www.cyberciti.biz/faq/freebsd-install-php-7-2-with-fpm-for-nginx/)

## Install php and php-fpm 

Install php and php-extensions:

`pkg install php72 php72-extensions`

Config php-fpm:

`vim /usr/local/etc/php-fpm.d/www.conf`

Find line: `listen = 127.0.0.1:9000`

Update it as follows: `listen = /var/run/php72-fpm.sock`

Uncomment the following line:
```
listen.owner = www
listen.group = www
listen.mode = 0660
```

Enable php-fpm service:

`sysrc php_fpm_enable=YES`

Start service:

`service php-fpm start`

## Hide php version

Create config file:

`cp -v /usr/local/etc/php.ini-production /usr/local/etc/php.ini`

Edit config file:

`vim /usr/local/etc/php.ini`

Find line: `expose_php = On`

and turn it Off: `expose_php = Off`

Restart service:

`service php-fpm restart`

## Nginx enable php-fpm

In nginx.conf:

```nginx
location ~ [^/]\.php(/|$) {
    root   /var/www/vhost2.com; # For example
    fastcgi_split_path_info ^(.+?\.php)(/.*)$;
    if (!-f $document_root$fastcgi_script_name) {
            return 404;
    }

    # Mitigate https://httpoxy.org/ vulnerabilities
        fastcgi_param HTTP_PROXY "";

    fastcgi_pass unix:/var/run/php72-fpm.sock;
    fastcgi_index index.php;

    # include the fastcgi_param setting
    include fastcgi_params;

    # SCRIPT_FILENAME parameter is used for PHP FPM determining
    # the script name.
    fastcgi_param  SCRIPT_FILENAME   $document_root$fastcgi_script_name;
}
```

Service reload:

`service nginx reload`