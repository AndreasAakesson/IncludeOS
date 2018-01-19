### httpLB demo

Start the nodeJS demo services first:
```
node server.js
```

Build and run the load balancer:
```
boot . --create-bridge
```

Connect to the load balancer:
```
curl 10.0.0.42:8080
```

