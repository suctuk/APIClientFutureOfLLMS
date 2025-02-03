const express = require('express');
const app = express();

// Add CORS headers
app.use((req, res, next) => {
    res.header('Access-Control-Allow-Origin', '*');
    res.header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.header('Access-Control-Allow-Headers', 'Origin, X-Requested-With, Content-Type, Accept');
    next();
});

// Add body parser middleware
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Add error handling middleware
app.use((err, req, res, next) => {
    console.error(err.stack);
    res.status(500).json({ error: 'Something went wrong!' });
});

// stores key value pairs. keys are usernames and values are messages
let messages = {
    "admin": "TOP SECRET INFO: ASD*!HESHAUDHUASDH@*!",
    "user1": "",
};

// Add request logging
app.use((req, res, next) => {
    console.log(`${req.method} ${req.path}`, req.body || req.query);
    next();
});

app.post("/create_user", (req, res) => {
    try {
        const username = req.body.username;
        if (!username) {
            return res.status(400).json({ error: 'Username is required' });
        }
        
        console.log('Creating user:', username);
        messages[username] = "";
        res.status(200).json({ message: 'User created successfully' });
    } catch (error) {
        console.error('Error creating user:', error);
        res.status(500).json({ error: 'Failed to create user' });
    }
});

app.post("/send_message", (req, res) => {
    try {
        const { sendto, message } = req.body;
        if (!sendto || !message) {
            return res.status(400).json({ error: 'Recipient and message are required' });
        }
        
        if (sendto.toLowerCase() === 'all') {
            // Broadcast to all users
            for (let user in messages) {
                messages[user] = message;
            }
            console.log('Broadcasting message to all users:', message);
            res.status(200).json({ message: 'Message broadcast successfully' });
            return;  // Important: return here to avoid the next check
        }
        
        // Send to specific user
        if (!(sendto in messages)) {
            return res.status(404).json({ error: 'Recipient not found' });
        }
        
        console.log(`Sending message to ${sendto}:`, message);
        messages[sendto] = message;
        res.status(200).json({ message: 'Message sent successfully' });
    } catch (error) {
        console.error('Error sending message:', error);
        res.status(500).json({ error: 'Failed to send message' });
    }
});

app.get("/latest_message", (req, res) => {
    console.log(messages);
    try {
        const username = req.query.username;
        if (!username) {
            return res.status(400).json({ error: 'Username is required' });
        }
        
        if (!(username in messages)) {
            return res.status(404).json({ error: 'User not found' });
        }
        
        console.log(`Retrieving message for ${username}:`, messages[username]);
        res.status(200).json({ message: messages[username] });
    } catch (error) {
        console.error('Error retrieving message:', error);
        res.status(500).json({ error: 'Failed to retrieve message' });
    }
});

// Add this endpoint to get all users
app.get("/users", (req, res) => {
    try {
        const userList = Object.keys(messages);
        res.status(200).json({ users: userList });
    } catch (error) {
        console.error('Error getting users:', error);
        res.status(500).json({ error: 'Failed to get users' });
    }
});

// Define a test endpoint
app.get('/test', (req, res) => {
    res.json({ message: 'hello world' });
});

// Start the server
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
    console.log(`Server is running on port ${PORT}`);
});
