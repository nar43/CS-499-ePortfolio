package com.zybooks.eventtracking;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

public class MainActivity extends AppCompatActivity {

    private DBHelper dbHelper;
    private EditText editTextUsername, editTextPassword;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);

        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        dbHelper = new DBHelper(this);

        editTextUsername = findViewById(R.id.editTextUsername);
        editTextPassword = findViewById(R.id.editTextPassword);
        Button buttonLogin = findViewById(R.id.buttonLogin);
        Button buttonCreateAccount = findViewById(R.id.buttonCreateAccount);

        buttonLogin.setOnClickListener(v -> loginUser());
        buttonCreateAccount.setOnClickListener(v -> createAccount());
    }

    // ✅ TODO (Enhancement 1): Add input validation before allowing login or account creation
    private boolean validateInputs(String username, String password) {
        if (username.isEmpty() || password.isEmpty()) {
            Toast.makeText(this, "⚠️ Username and password cannot be empty.", Toast.LENGTH_SHORT).show();
            return false;
        }
        if (username.length() < 3) {
            Toast.makeText(this, "⚠️ Username must be at least 3 characters.", Toast.LENGTH_SHORT).show();
            return false;
        }
        if (password.length() < 4) {
            Toast.makeText(this, "⚠️ Password must be at least 4 characters.", Toast.LENGTH_SHORT).show();
            return false;
        }
        return true;
    }

    private void loginUser() {
        String username = editTextUsername.getText().toString().trim();
        String password = editTextPassword.getText().toString().trim();

        if (!validateInputs(username, password)) return; // ✅ Enhanced validation

        SQLiteDatabase db = dbHelper.getReadableDatabase();
        Cursor cursor = db.rawQuery("SELECT password FROM users WHERE username = ?", new String[]{username});

        if (cursor.moveToFirst()) {
            String storedPassword = cursor.getString(0);
            if (storedPassword.equals(password)) {
                Toast.makeText(this, "✅ Login successful", Toast.LENGTH_SHORT).show();
                // TODO: Navigate to next screen (DataDisplayActivity)
            } else {
                Toast.makeText(this, "❌ Incorrect password", Toast.LENGTH_SHORT).show();
            }
        } else {
            Toast.makeText(this, "❌ User does not exist", Toast.LENGTH_SHORT).show();
        }
        cursor.close();
    }

    private void createAccount() {
        String username = editTextUsername.getText().toString().trim();
        String password = editTextPassword.getText().toString().trim();

        if (!validateInputs(username, password)) return; // ✅ Enhanced validation

        SQLiteDatabase db = dbHelper.getWritableDatabase();
        Cursor cursor = db.rawQuery("SELECT username FROM users WHERE username = ?", new String[]{username});

        if (cursor.moveToFirst()) {
            Toast.makeText(this, "⚠️ Username already exists", Toast.LENGTH_SHORT).show();
        } else {
            ContentValues values = new ContentValues();
            values.put("username", username);
            values.put("password", password);

            long result = db.insert("users", null, values);
            if (result != -1) {
                Toast.makeText(this, "✅ Account created successfully", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "❌ Error creating account", Toast.LENGTH_SHORT).show();
            }
        }
        cursor.close();
    }
}
