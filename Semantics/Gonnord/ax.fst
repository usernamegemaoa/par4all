//Generated by c2fsm -s 
model ax {
var i,j,n,tmp,tmp_1;
//@parameters n;
states stop,start,lbl_4,lbl_7,lbl_4_1;
transition t_18 :={
  from  := start;
  to    := lbl_4_1;
  guard := true;
  action:= i' = 0;
};
transition tp_1 :={
  from  := lbl_4;
  to    := lbl_4_1;
  guard := ( (i+2 <= n) && (n <= j+1) );
  action:=;
};
transition t_11 :={
  from  := lbl_4;
  to    := stop;
  guard := ( (n <= i+1) || (j+2 <= n) );
  action:=;
};
transition t_13 :={
  from  := lbl_7;
  to    := lbl_4;
  guard := (n <= j+1);
  action:= i' = i+1, tmp' = i;
};
transition t_16 :={
  from  := lbl_7;
  to    := lbl_7;
  guard := (j+2 <= n);
  action:= j' = j+1, tmp_1' = j;
};
transition t_17 :={
  from  := lbl_4_1;
  to    := lbl_7;
  guard := true;
  action:= j' = 0;
};
}
strategy dumb {
    Region init := { state = start && (0 < n) };
}
