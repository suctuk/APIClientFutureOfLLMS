const express = require('express');
const app = express();

// Define a test endpoint
app.get('/test', (req, res) => {
  res.send('hello world');
});

// Start the server
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});
