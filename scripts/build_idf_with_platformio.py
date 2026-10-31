"""Supplementary IDF 6.1 compile check in isolated copies, using PlatformIO.

This does not certify native idf.py installation. Original examples remain
ordinary ESP-IDF projects; only copies gain a temporary PlatformIO manifest.
"""
import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--work-root',type=Path,required=True)
    parser.add_argument('--match',default='')
    parser.add_argument('--output',type=Path,default=ROOT/'validation'/f'idf-pio-{platform.system().lower()}.json')
    parser.add_argument('--timeout',type=int,default=1200)
    parser.add_argument('--retry-failed',action='store_true')
    args=parser.parse_args()
    records=json.loads(args.output.read_text(encoding='utf-8')) if args.retry_failed and args.output.exists() else []
    for original in sorted((ROOT/'examples/esp-idf').iterdir()):
        if not original.is_dir() or args.match not in original.name: continue
        name=original.relative_to(ROOT).as_posix()
        old=next((r for r in records if r['project']==name),None)
        if args.retry_failed and old and old['status']=='passed': continue
        # Keep a deliberate short Windows junction path. resolve() expands it
        # and can exceed cmd.exe's command-line limit in IDF link generation.
        target=args.work_root.absolute()/original.name
        target.mkdir(parents=True,exist_ok=True)
        # Overwrite only source copies owned by this validation workspace.
        for source in original.rglob('*'):
            if source.is_file() and not any(p in source.relative_to(original).parts for p in ('build','.pio','managed_components')) and source.name not in ('sdkconfig','sdkconfig.old','server_root.h'):
                destination=target/source.relative_to(original)
                destination.parent.mkdir(parents=True,exist_ok=True)
                shutil.copy2(source,destination)
        config='[platformio]\nsrc_dir = main\n[env:esp32dev]\nplatform = espressif32@8.0.0\nboard = esp32dev\nframework = espidf\n'
        if (target/'partitions.csv').exists(): config+='board_build.partitions = partitions.csv\n'
        (target/'platformio.ini').write_text(config,encoding='utf-8')
        command=[sys.executable,'-m','platformio','run','-d',str(target)]
        args.output.parent.mkdir(parents=True,exist_ok=True)
        logdir=args.output.parent/'logs';logdir.mkdir(exist_ok=True)
        log=logdir/f'idf-pio-{platform.system().lower()}-{original.name}.log'
        record={'project':name,'host':platform.platform(),'method':'PlatformIO 8.0.0 managed ESP-IDF 6.1; isolated copy','hardware_tested':False,'started_utc':datetime.now(timezone.utc).isoformat()}
        print('BUILD IDF',name,flush=True)
        try:
            result=subprocess.run(command,capture_output=True,timeout=args.timeout,cwd=target)
            log.write_bytes(result.stdout+b'\n'+result.stderr)
            record.update(status='passed' if result.returncode==0 else 'failed',exit_code=result.returncode,log=log.name)
        except subprocess.TimeoutExpired as exc:
            log.write_bytes((exc.stdout or b'')+(exc.stderr or b''))
            record.update(status='timeout',log=log.name)
        records=[r for r in records if r['project']!=name]+[record]
        args.output.write_text(json.dumps(records,ensure_ascii=False,indent=2),encoding='utf-8')
        print(record['status'],flush=True)
    return 0 if records and all(r['status']=='passed' for r in records) else 1

if __name__=='__main__': raise SystemExit(main())
