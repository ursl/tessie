# How to run the server

- stop apache
- install dependencies
  ```
  sudo apt install npm
  sudo apt install nodejs
  ```
- install pacakges
  ```
  cd tessie/node/test1
  npm install --save express socket.io mqtt
  ```
- run the server
  ```
  cd tessie/node/test1
  node server3.js
  ```
- Alternatively run it with `pm2` or by adding to an autostart setup, e.g., 
  ```
  // -- .config/autostart/tessieWeb.desktop
  [Desktop Entry]
  Name=TessieWeb
  Exec="cd /home/pi/tessie/node/test1 && node server3.js"
  ```
  or with `systemctl` (as described in the main tessie README)
- Connect to this server with  http://coldbox01:3000
  NOTE: http, not https!
  
