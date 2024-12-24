// firebase.js
import { initializeApp } from "https://www.gstatic.com/firebasejs/11.0.2/firebase-app.js";
import { getDatabase, ref, set } from "https://www.gstatic.com/firebasejs/11.0.2/firebase-database.js";

const firebaseConfig = {
  apiKey: "AIzaSyBwfoFcJx-YxBigNRrzvcvshP58UKOjekg",
  authDomain: "parkingsystem-9d434.firebaseapp.com",
  databaseURL: "https://parkingsystem-9d434-default-rtdb.firebaseio.com",
  projectId: "parkingsystem-9d434",
  storageBucket: "parkingsystem-9d434.firebasestorage.app",
  messagingSenderId: "637394675226",
  appId: "1:637394675226:web:45d26f0eab5653290893a3",
  measurementId: "G-KCLQ2KKZ4E"
};

const app = initializeApp(firebaseConfig);
const database = getDatabase(app);

export { database, ref, set };
