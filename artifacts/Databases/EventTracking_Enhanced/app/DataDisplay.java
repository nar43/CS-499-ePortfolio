package com.zybooks.eventtracking;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

public class DataDisplayActivity extends AppCompatActivity {

    private DBHelper dbHelper;
    private TableLayout tableLayout;
    private EditText inputName, inputDate;
    private Button buttonAdd;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_data_display);

        dbHelper = new DBHelper(this);

        tableLayout = findViewById(R.id.tableLayout);
        buttonAdd = findViewById(R.id.buttonAdd);
        inputName = new EditText(this);
        inputDate = new EditText(this);

        buttonAdd.setOnClickListener(v -> showAddEventDialog());

        loadEvents();
    }

    private void loadEvents() {
        tableLayout.removeViews(1, Math.max(0, tableLayout.getChildCount() - 1)); // Clear old rows

        SQLiteDatabase db = dbHelper.getReadableDatabase();
        Cursor cursor = db.rawQuery("SELECT * FROM events", null);

        while (cursor.moveToNext()) {
            int id = cursor.getInt(cursor.getColumnIndexOrThrow("id"));
            String name = cursor.getString(cursor.getColumnIndexOrThrow("name"));
            String date = cursor.getString(cursor.getColumnIndexOrThrow("date"));

            TableRow row = new TableRow(this);
            TextView nameView = new TextView(this);
            TextView dateView = new TextView(this);
            Button deleteButton = new Button(this);

            nameView.setText(name);
            dateView.setText(date);
            deleteButton.setText("Delete");

            row.addView(nameView);
            row.addView(dateView);
            row.addView(deleteButton);

            deleteButton.setOnClickListener(v -> {
                SQLiteDatabase writeDB = dbHelper.getWritableDatabase();
                writeDB.delete("events", "id=?", new String[]{String.valueOf(id)});
                loadEvents();
            });

            tableLayout.addView(row);
        }

        cursor.close();
    }

    private void showAddEventDialog() {
        // Simple input method using inline textboxes
        inputName.setHint("Event Name");
        inputDate.setHint("Date (YYYY-MM-DD)");

        inputName.setText("");
        inputDate.setText("");

        TableRow inputRow = new TableRow(this);
        inputRow.addView(inputName);
        inputRow.addView(inputDate);

        Button confirmButton = new Button(this);
        confirmButton.setText("Save");
        inputRow.addView(confirmButton);

        tableLayout.addView(inputRow);

        confirmButton.setOnClickListener(v -> {
            String name = inputName.getText().toString().trim();
            String date = inputDate.getText().toString().trim();

            if (!name.isEmpty() && !date.isEmpty()) {
                SQLiteDatabase db = dbHelper.getWritableDatabase();
                ContentValues values = new ContentValues();
                values.put("name", name);
                values.put("date", date);
                db.insert("events", null, values);
                loadEvents();
                tableLayout.removeView(inputRow);
            }
        });
    }
}
