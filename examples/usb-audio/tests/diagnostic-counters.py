#!/usr/bin/env python3
"""Query a muted USB audio diagnostic's counters via CC22; no counter reset."""
import argparse,json,mido,pathlib,subprocess,sys,time
FIELDS='rate starts packets frames underruns overruns missed stalls adjust_up adjust_down fill_low fill_high fill_current cpu_percent audio_xruns rate_sets alt'.split()
p=argparse.ArgumentParser();p.add_argument('--port',required=True);p.add_argument('--token',type=int,choices=range(128),default=37);p.add_argument('--output',type=pathlib.Path);a=p.parse_args()
r=subprocess.run([sys.executable,str(pathlib.Path(__file__).with_name('usb-descriptors.py')),'--product','FM1 USB Audio Test'],capture_output=True,text=True,check=True)
assert json.loads(r.stdout)['product']=='FM1 USB Audio Test'
assert a.port in mido.get_input_names() and a.port in mido.get_output_names()
with mido.open_input(a.port) as source,mido.open_output(a.port) as dest:
 dest.send(mido.Message('control_change',control=22,value=a.token))
 deadline=time.monotonic()+3
 while time.monotonic()<deadline:
  for msg in source.iter_pending():
   if msg.type!='sysex':continue
   data=list(msg.data)
   if data[:5]!=[0x7d,0x55,0x41,1,a.token]:continue
   assert len(data)==90, f'unexpected payload size {len(data)}'
   report={name:sum(data[5+i*5+j]<<(7*j) for j in range(5)) for i,name in enumerate(FIELDS)}
   output=json.dumps(report,indent=2)+'\n'
   if a.output:a.output.write_text(output)
   print(output,end='');raise SystemExit(0)
  time.sleep(.01)
raise SystemExit('No diagnostic counter reply within 3 seconds')
