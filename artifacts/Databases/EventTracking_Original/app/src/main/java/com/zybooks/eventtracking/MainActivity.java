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

        // Make sure layout root in activity_main.xml has android:id="@+id/main"
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

    private void loginUser() {
        String username = editTextUsername.getText().toString().trim();
        String password = editTextPassword.getText().toString().trim();

        SQLiteDatabase db = dbHelper.getReadableDatabase();
        Cursor cursor = db.rawQuery("SELECT password FROM users WHERE username = ?", new String[]{username});

        if (cursor.moveToFirst()) {
            String storedPassword = cursor.getString(0);
            if (storedPassword.equals(password)) {
                Toast.makeText(this, "✅ Login successful", Toast.LENGTH_SHORT).show();
                // TODO: Navigate to the next screen (e.g., DataDisplayActivity)
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