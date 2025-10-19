package com.zybooks.eventtracking;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.telephony.SmsManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

public class SMSActivity extends AppCompatActivity {

    private static final int SMS_PERMISSION_CODE = 100;
    private TextView textViewSMSStatus;
    private Button buttonRequestPermission;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sms);

        textViewSMSStatus = findViewById(R.id.textViewSMSStatus);
        buttonRequestPermission = findViewById(R.id.buttonRequestPermission);

        checkPermission();

        buttonRequestPermission.setOnClickListener(v -> {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.SEND_SMS) != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.SEND_SMS}, SMS_PERMISSION_CODE);
            } else {
                sendNotificationSMS();
            }
        });
    }

    private void checkPermission() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.SEND_SMS) == PackageManager.PERMISSION_GRANTED) {
            textViewSMSStatus.setText("SMS permission is already granted ✅");
        } else {
            textViewSMSStatus.setText("SMS permission not granted ❌");
        }
    }

    private void sendNotificationSMS() {
        try {
            SmsManager sms = SmsManager.getDefault();
            sms.sendTextMessage("5554", null, "Reminder: Upcoming event this week!", null, null);
            Toast.makeText(this, "SMS sent!", Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
            Toast.makeText(this, "SMS failed: " + e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if (requestCode == SMS_PERMISSION_CODE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                textViewSMSStatus.setText("Permission granted ✅");
                sendNotificationSMS();
            } else {
                textViewSMSStatus.setText("Permission denied ❌");
            }
        }
    }
}
