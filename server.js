const express = require('express');
const app = express();



// stores key value pairs. keys are usernames and values are messages
let messages = {
  "admin" : "TOP SECRET INFO: ASD*!HESHAUDHUASDH@*!",
  "user1" : "",
};


app.post("/create_user", (req, res) => {
  let username = req.body.username;
  messages[username] = "";
  res.sendStatus(200);
});

app.post("/send_message", (req, res) => {
  let sendto = req.body.sendto;
  let message = req.body.message;
  messages[sendto] = message;
  res.sendStatus(200);
});

app.get("/latest_message", (req, res) => {
  let username = req.query.username;
  res.send(messages[username]);
});

// Define a test endpoint
app.get('/test', (req, res) => {
  res.send('hello world');
});

// Start the server
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});
