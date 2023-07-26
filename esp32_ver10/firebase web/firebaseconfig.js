<script type="module">
            // Import the functions you need from the SDKs you need
            import { initializeApp } from "https://www.gstatic.com/firebasejs/10.1.0/firebase-app.js";
            import { getAnalytics } from "https://www.gstatic.com/firebasejs/10.1.0/firebase-analytics.js";
            // TODO: Add SDKs for Firebase products that you want to use
            // https://firebase.google.com/docs/web/setup#available-libraries
          
            // Your web app's Firebase configuration
            // For Firebase JS SDK v7.20.0 and later, measurementId is optional
            const firebaseConfig = {
              apiKey: "AIzaSyD0UmlJNoacdrBfyZ50h0FyEI0yHKWUwBs",
              authDomain: "user-data-2e9be.firebaseapp.com",
              databaseURL: "https://user-data-2e9be-default-rtdb.firebaseio.com",
              projectId: "user-data-2e9be",
              storageBucket: "user-data-2e9be.appspot.com",
              messagingSenderId: "261163235969",
              appId: "1:261163235969:web:c60872f373060f75cb2b80",
              measurementId: "G-2X855JVZ8V"
            };
          
            // Initialize Firebase
            const app = initializeApp(firebaseConfig);
            const analytics = getAnalytics(app);

            import { getDatabase, set, get, update, remove, ref, child } from "https://www.gstatic.com/firebasejs/10.1.0/firebase-database.js";
            const db = getDatabase();

            var account = document.querySelector("#account");
            var password = document.querySelector("#password");
            var loginBtn = document.querySelector("#login");
            var findAccount = document.querySelector("#findAccount");
            var findPassword = document.querySelector("#findPassword");
            var findPhoneNumber = document.querySelector("#findPhoneNumber");
            var findEmail = document.querySelector("#findEmail");

            function loginData() {
                const dbref = ref(db);
                get(child(dbref, "UserData/使用者" + account.value))
                .then((snapshot)=>{
                    if (snapshot.exists()) {
                        const Password = '"' + password.value + '"'
                        if (Password == snapshot.val().password){
                            alert("登入成功")
                        }else{
                            alert("密碼錯誤")
                        }
                    }else{
                        alert("帳號不存在")
                    }
                })
            }

            loginBtn.addEventListener('click', loginData);

          </script>