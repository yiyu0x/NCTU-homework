# HTTP Applications

nginx.conf:
```nginx
server {
    listen       80;
    server_name  app.com;
    root         /var/www/app;
    index        index.html index.htm index.php;
    location /app {
        try_files $uri $uri/ /index.php$is_args$args;
    }
    location ~ \.php$ {
        try_files $uri =404;
        include fastcgi_params;
        fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
        fastcgi_pass unix:/var/run/php72-fpm.sock;
        fastcgi_index index.php;
    }
}
```

index.php:
```php
<?php
    if (isset($_GET['name'])) {
        echo "Hello, ".$_GET['name'];
    } else {
        $req_url = $_SERVER[REQUEST_URI];
        if ($req_url == "/app") {
            echo "route: /";
        } else {
            $matches = array();
            preg_match('/^\/app\/([0-9]+)\+([0-9]+)$/', $req_url, $matches);
            $sum = (int)$matches[1] + (int)$matches[2];
            echo "result: $sum";
        }
    }
?>
```