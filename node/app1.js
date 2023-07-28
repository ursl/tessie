const express = require('express');
const app = express();
const port = 80

app.get('/', (req, res) => {
    res.send('app1: Hello World!')
})


app.listen(port, () => {
  console.log(`app1 listening on port ${port}`)
})


app.get('/tessie/', (req, res) => {
  res.send('Got a tessie request')
})


