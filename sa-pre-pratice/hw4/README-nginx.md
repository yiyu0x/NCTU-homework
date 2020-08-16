# HW4

## HTTP Server

Install nginx:

`pkg install nginx`

Turn on nginx:

`sysrc nginx_enable="YES"`

or

`echo 'nginx_enable="YES"' >> /etc/rc.conf`

### Virtual Host

Edit:

`/usr/local/etc/nginx/nginx.conf`

Add follow content into http section:

```nginx
server {
    listen       80;
    server_name  vhost1.com;

    location / {
        root   /var/www/vhost1.com;
        index  index.html index.htm;
    }
}
```

```nginx
    server {
        listen       80;
        server_name  172.16.85.147;

        location / {
            root   /var/www/172.16.85.147;
            index  index.html index.htm;
        }
    }
```

### Access Control

#### Basic Auth

Generate .htpasswd hash:

`openssl passwd -apr1`

Assume output is:

`$apr1$O6Y3y6Ut$EP3jlhDhiWAliV2xzyt0c0`

Create a .hpasswd file and add:

`yiyu:$apr1$O6Y3y6Ut$EP3jlhDhiWAliV2xzyt0c0`

filed one is your basic auth username

Add basic auth info into `/usr/local/etc/nginx/nginx.conf` http -> server -> location section

```nginx
auth_basic  "my Administrator";
auth_basic_user_file /usr/local/etc/nginx/.htpasswd;
```

#### Restricting Access by IP Address

```nginx
server {
    listen 12345;
    deny   192.168.1.2;
    allow  192.168.1.1/24;
    allow  2001:0db8::/32;
    deny   all;
}
```

### Hiding Server Information

Add follow content into http section:

`server_tokens off;`

### HTTPS

Self-signed certificate:

`openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout /usr/local/etc/ssl/nginx.key -out /usr/local/etc/ssl/nginx.crt`

Ceate a self-signed.conf and edit:

`vim /usr/local/etc/nginx/snippets/self-signed.conf`

```nginx
ssl_certificate /usr/local/etc/ssl/nginx.crt;
ssl_certificate_key /usr/local/etc/ssl/nginx.key;
```

Edit nginx.conf

```nginx
server {
    listen       443 ssl;
    server_name  vhost1.com;
    include      snippets/self-signed.conf;

    location / {
        root   /var/www/vhost1.com;
        index  index.html index.htm;
    }
}
```

### Connect HTTP Redirect to HTTPS

```nginx
server {
    listen       80;
    server_name  vhost1.com;
    return 302 https://$server_name$request_uri;
#   location / {
#       root   /var/www/vhost1.com;
#       index  index.html index.htm;
#   }

}

server {
    listen       443 ssl;
    server_name  vhost1.com;
    include snippets/self-signed.conf;

    location / {
        root   /var/www/vhost1.com;
        index  index.html index.htm;
    }
}
```

### HTTP2

```nginx
server {
    listen       443 ssl http2;
    server_name  vhost1.com;
    include snippets/self-signed.conf;

    location / {
        root   /var/www/vhost1.com;
        index  index.html index.htm;
    }
}
```