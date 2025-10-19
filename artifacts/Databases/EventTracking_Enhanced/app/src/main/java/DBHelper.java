package com.zybooks.eventtracking;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class DBHelper extends SQLiteOpenHelper {

    private static final String DATABASE_NAME = "EventTracking.db";
    private static final int DATABASE_VERSION = 2; // ✅ Incremented for upgrade

    public DBHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        // Create a table for users with username as primary key
        String createUsersTable = "CREATE TABLE users (" +
                "username TEXT PRIMARY KEY, " +
                "password TEXT)";
        db.execSQL(createUsersTable);

        // Create a table for events
        String createEventsTable = "CREATE TABLE events (" +
                "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                "name TEXT NOT NULL, " +
                "date TEXT NOT NULL)";
        db.execSQL(createEventsTable);

        // ✅ TODO (Enhancement 2): Add offline sync queue table
        String createPendingSyncTable = "CREATE TABLE pending_sync (" +
                "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                "name TEXT NOT NULL, " +
                "date TEXT NOT NULL)";
        db.execSQL(createPendingSyncTable);

        // ✅ TODO (Enhancement 3): Add indexes for faster queries
        db.execSQL("CREATE INDEX idx_event_name ON events(name)");
        db.execSQL("CREATE INDEX idx_event_date ON events(date)");
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        db.execSQL("DROP TABLE IF EXISTS users");
        db.execSQL("DROP TABLE IF EXISTS events");
        db.execSQL("DROP TABLE IF EXISTS pending_sync");
        onCreate(db);
    }

    // ✅ TODO (Enhancement 2): Simulated offline insert
    public void insertEventOffline(String name, String date) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put("name", name);
        values.put("date", date);
        db.insert("pending_sync", null, values);
        Log.d("DBHelper", "Event stored offline: " + name);
    }

    // ✅ TODO (Enhancement 2): Simulate sync when online
    public void syncPendingEvents() {
        SQLiteDatabase db = this.getWritableDatabase();
        Cursor cursor = db.rawQuery("SELECT * FROM pending_sync", null);

        while (cursor.moveToNext()) {
            ContentValues values = new ContentValues();
            values.put("name", cursor.getString(cursor.getColumnIndexOrThrow("name")));
            values.put("date", cursor.getString(cursor.getColumnIndexOrThrow("date")));
            db.insert("events", null, values);
        }

        db.execSQL("DELETE FROM pending_sync"); // clear synced events
        cursor.close();
        Log.d("DBHelper", "✅ Pending events synced successfully");
    }
}
