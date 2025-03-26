package com.example.lms_21;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class Settings extends AppCompatActivity {
    TextView mainread;
    FirebaseDatabase db = FirebaseDatabase.getInstance();
    DatabaseReference node, node2, setunitsnode,ampsnode, powernode, voltnode, dcnode, unitsnode, freqnode, pfnode,modenode;
    private Switch loadSW;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        mainread = (TextView) findViewById(R.id.tagline);
        TextView amps = (TextView) findViewById(R.id.amps);
        TextView power = (TextView) findViewById(R.id.power);
        TextView volt = (TextView) findViewById(R.id.acvolt);
        TextView dcvolt = (TextView) findViewById(R.id.ph3volt);
        TextView freq = (TextView) findViewById(R.id.freq);
        TextView pf = (TextView) findViewById(R.id.pf);
        TextView units = (TextView) findViewById(R.id.units);
        TextView Setunit = (TextView) findViewById(R.id.unitsSet);
        EditText setunits = (EditText) findViewById(R.id.unitSet);
        Button subBtn = (Button) findViewById(R.id.SubmitBtn);
        loadSW = (Switch) findViewById(R.id.switch1);

        node2 = db.getReference("UserInput");
        setunitsnode = node2.child("units");
        modenode = node2.child("mode");

        node = db.getReference("Data");
        unitsnode = node.child("units");
        ampsnode = node.child("current");
        powernode = node.child("power");
        voltnode = node.child("voltage");
        dcnode = node.child("phvolt");
        freqnode = node.child("freq");
        pfnode = node.child("pf");

        ampsnode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue(String.class);
                amps.setText(value+" A");
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Toast.makeText(Settings.this, "Connection Error", Toast.LENGTH_LONG).show();
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
                Toast.makeText(Settings.this, "Connection Error", Toast.LENGTH_LONG).show();
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
                Toast.makeText(Settings.this, "Connection Error", Toast.LENGTH_LONG).show();
            }
        });
        dcnode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue(String.class);
                dcvolt.setText(value+" V");
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Toast.makeText(Settings.this, "Connection Error", Toast.LENGTH_LONG).show();
            }
        });
        freqnode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue().toString();
                freq.setText(value+" Hz");
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Toast.makeText(Settings.this, "Connection Error", Toast.LENGTH_LONG).show();
            }
        });
        pfnode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue().toString();
                pf.setText(value);
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Toast.makeText(Settings.this, "Connection Error", Toast.LENGTH_LONG).show();
            }
        });
        unitsnode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue(String.class);

                units.setText(value+" KWh");
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Toast.makeText(Settings.this, "Connection Error", Toast.LENGTH_LONG).show();
            }
        });
        setunitsnode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue().toString();
                Setunit.setText(value+" KWh");
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Toast.makeText(Settings.this, "Connection Error", Toast.LENGTH_LONG).show();
            }
        });

        subBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String units = setunits.getText().toString();
                if (units.isEmpty()) {
                    setunits.setError("Set Units cannot be empty");
                    setunits.requestFocus();
                    return;
                }
                node2.child("units").setValue(units);
                node2.child("reset").setValue("1");
                Toast.makeText(Settings.this, "Success !", Toast.LENGTH_LONG).show();
            }
        });
        modenode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue().toString();
                if(value.equals("1")) {
                    loadSW.setChecked(true);
                }
                else if(value.equals("0")){
                    loadSW.setChecked(false);
                }
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Log.e("Error: ", String.valueOf(error));
            }
        });
        loadSW.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    node2.child("mode").setValue("1");
                } else {
                    node2.child("mode").setValue("0");
                }
            }
        });

        FirebaseDatabase.getInstance().getReference(".info/connected").addValueEventListener(new ValueEventListener() {
            public void onDataChange(DataSnapshot snapshot) {
                if (((Boolean) snapshot.getValue(Boolean.class)).booleanValue()) {
                    Settings.this.mainread.setText("INTERNET CONNECTED ");
                } else {
                    Settings.this.mainread.setText("INTERNET CONNECTION FAILED");
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