module.exports = function(app){
  app.use('/', require('./routes/home'));
  app.use('/about', require('./routes/about'));
}
