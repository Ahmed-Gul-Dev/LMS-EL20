package com.example.lms_21;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class Load2 extends AppCompatActivity {
    TextView mainread;
    FirebaseDatabase db = FirebaseDatabase.getInstance();
    DatabaseReference node,node2,ampsnode,powernode,voltnode,dcnode,loadnode;
    private ImageView img;
    private Switch load2SW;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_load1);

        TextView amps = (TextView) findViewById(R.id.amps);
        mainread = (TextView) findViewById(R.id.tagline);
        TextView power = (TextView) findViewById(R.id.power);
        TextView volt = (TextView) findViewById(R.id.acvolt);
        img = (ImageView) findViewById(R.id.imageView1);
        load2SW = (Switch) findViewById(R.id.switch1);

        node2 = db.getReference("UserInput");
        node = db.getReference("Data");
        ampsnode = node.child("amp2");
        powernode = node.child("pow2");
        voltnode = node.child("voltage");
        loadnode = node.child("load2state");

        ampsnode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue(String.class);
                amps.setText(value+" A");
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Toast.makeText(Load2.this, "Connection Error", Toast.LENGTH_LONG).show();
            }
        });
        voltnode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue(String.class);
                volt.setText(value+" V");
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Toast.makeText(Load2.this, "Connection Error", Toast.LENGTH_LONG).show();
            }
        });
        powernode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue(String.class);
                power.setText(value+" W");
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Toast.makeText(Load2.this, "Connection Error", Toast.LENGTH_LONG).show();
            }
        });
        loadnode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue(String.class); //load status
                if(value.equals("ON")) {
                    img.setImageResource(R.drawable.on);
//                    load1SW.setChecked(true);
                }
                else if(value.equals("OFF")){
                    img.setImageResource(R.drawable.off);
//                    load1SW.setChecked(false);
                }
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Log.e("Error: ", String.valueOf(error));
            }
        });
        load2SW.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    node2.child("load2").setValue("ON");
                } else {
                    node2.child("load2").setValue("OFF");
                }
            }
        });

        FirebaseDatabase.getInstance().getReference(".info/connected").addValueEventListener(new ValueEventListener() {
            public void onDataChange(DataSnapshot snapshot) {
                if (((Boolean) snapshot.getValue(Boolean.class)).booleanValue()) {
                    Load2.this.mainread.setText("INTERNET CONNECTED ");
                } else {
                    Load2.this.mainread.setText("INTERNET CONNECTION FAILED");
                }
            }

            public void onCancelled(DatabaseError error) {
            }
        });
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        finish();
    }
}