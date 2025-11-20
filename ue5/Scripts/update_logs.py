# //// BEGIN NEW CODE
#!/usr/bin/env python3
"""Utility to append dev log entries for Only The Dead."""

import argparse
from datetime import datetime
from pathlib import Path

LOGS_DIR = Path(__file__).resolve().parents[1] / "Logs"
WORK_LOG = LOGS_DIR / "OTD_WorkLog.md"
PROGRESS_LOG = LOGS_DIR / "OTD_DevProgress.md"

WORK_TEMPLATE = "- [{time}] ({phase}) {summary}\n"
PROGRESS_TEMPLATE = "| {time} | {phase} | {task} | {status} | {notes} |\n"


def append_work_entry(phase: str, summary: str) -> None:
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M %z")
    entry = WORK_TEMPLATE.format(time=timestamp, phase=phase, summary=summary)
    with WORK_LOG.open("a", encoding="utf-8") as fh:
        fh.write(entry)


def append_progress_entry(phase: str, task: str, status: str, notes: str) -> None:
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M %z")
    entry = PROGRESS_TEMPLATE.format(
        time=timestamp,
        phase=phase,
        task=task,
        status=status,
        notes=notes,
    )
    with PROGRESS_LOG.open("a", encoding="utf-8") as fh:
        fh.write(entry)


def main() -> None:
    parser = argparse.ArgumentParser(description="Append Only The Dead dev logs.")
    parser.add_argument("phase", help="Phase identifier, e.g., 'Phase 0'")
    parser.add_argument("task", help="Task description")
    parser.add_argument("status", help="Status (✅ Complete / 🚧 In Progress / etc.)")
    parser.add_argument("notes", help="Notes to record")
    parser.add_argument("summary", help="Short sentence for work log")
    args = parser.parse_args()

    append_work_entry(args.phase, args.summary)
    append_progress_entry(args.phase, args.task, args.status, args.notes)


if __name__ == "__main__":
    main()
# //// END NEW CODE
