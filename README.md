# CampusCompute: OS Level 3 Mini-Project

A command-line C prototype for Ubuntu Linux or WSL Ubuntu. It demonstrates scheduling simulations and Banker's safety check, plus real POSIX threads, child processes, shared memory, a named semaphore, and a process resource limit. Worker failure is deliberately simulated.

## 1. Prerequisites

Ubuntu, GCC, Make, and the standard POSIX development libraries. If GCC or Make is missing, install the Ubuntu package `build-essential`. You may also need `zip` and `unzip` for the submission archive.

```sh
sudo apt update
sudo apt install build-essential zip unzip
```

These commands can require an internet connection, an administrator password and permission to install packages. Check `gcc --version` and `make --version` first.

## 2. Build and run

Open an Ubuntu terminal in this directory, then:

```sh
make clean
make
./campuscompute help
```

Run commands **from the project directory**, since the integrated demo looks for the sample files there. You can modify the sample input files or create your own in the same format.

```sh
./campuscompute schedule fcfs jobs.txt
./campuscompute schedule rr jobs.txt
./campuscompute schedule priority jobs.txt
./campuscompute schedule compare jobs.txt
./campuscompute logger
./campuscompute ipc
./campuscompute banker safe.txt
./campuscompute banker unsafe.txt
./campuscompute worker normal
./campuscompute worker fail
./campuscompute limit
./campuscompute demo
./campuscompute nonsense    # deliberate invalid command; returns non-zero
```

`sh test.sh` builds the project and exercises each feature, including bad inputs. Thread order, PIDs, IPC object names and descriptor counts vary by run. The tested unsafe state is a successfully analyzed result, not a program failure.

## 3. Job input format

Plain text, whitespace-separated integers:

```text
number_of_jobs time_quantum
id arrival_time cpu_burst priority
... one line per job
```

`jobs.txt` has three jobs and quantum 2. IDs must be unique positive integers; arrival and priority must be 0..10000; burst and quantum must be 1..10000. Maximum 50 jobs. A priority with a smaller number is higher. For FCFS, equal arrivals use smaller ID; for priority scheduling, ties use earlier arrival then smaller ID. Round Robin queues simultaneous arrivals by arrival time then ID, and queues arrivals at a slice boundary before requeuing the interrupted job. Idle CPU intervals are displayed. FCFS and non-preemptive priority run a selected job to completion; RR uses a ready queue.

Times are simulation units. The three scheduling policies calculate a timeline; they do not replace or control the Linux CPU scheduler. Completion = finish time; turnaround = completion - arrival; waiting = turnaround - burst. No I/O bursts are modeled.

## 4. Banker input format

Plain text, whitespace-separated integers:

```text
number_of_processes number_of_resource_types
available for each resource
allocation matrix: one row per process
maximum matrix: one row per process
```

There may be 1..50 processes and 1..10 resources. Resource values must be 0..10000, with Allocation <= Maximum. The program calculates Need = Maximum - Allocation and runs the safety check; it shows a safe sequence or reports an unsafe state. Unsafe does not necessarily mean currently deadlocked. `safe.txt` and `unsafe.txt` exercise both outcomes.

## 5. Implementation notes and cleanup

- The logger uses one producer and one consumer, a three-entry bounded queue, a mutex and two condition variables. The producer signals that it is finished; the consumer drains the queue, then both threads are joined. `campuscompute.log` is opened in append mode, so previous log runs remain available for evidence.
- IPC uses a newly created, PID-specific POSIX shared-memory object and named semaphore. The parent writes; the child waits for the semaphore and reads. Both close/unmap their handles, the parent waits for the child, then unlinks the objects it created. Existing IPC objects are never unlinked by the program. Resource names are printed for verification; for example, on Linux, look for the printed names in `/dev/shm` after the command. Named semaphores usually have a `sem.` prefix there. If the program is forcibly killed, inspect the exact printed names before manually removing any leftover objects; do not remove unrelated objects.
- The worker demo uses `fork()` and `waitpid()`. `worker fail` deliberately exits with code 1 and is identified as simulated, not a real OS failure. The demo command itself succeeds when that expected failure is correctly detected.
- The limit demo applies `RLIMIT_NOFILE` inside a child only, then opens `/dev/null` until the limit is observed. It closes opened descriptors and the parent collects the child. Actual counts vary with the existing limit and descriptors.
- `make clean` removes this project's object files and executable. It leaves the log and source/input files intact. To remove the log when no longer needed, run `rm -f campuscompute.log` from this project directory.

## 6. Evidence and submission

Use your own full SRN in the **terminal prompt**, not just in file content:

```sh
export SRN='YOUR_FULL_SRN'
export PS1='[$SRN] \w\$ '
```

Replace the placeholder and ensure it is visible alongside the command and output in every genuine screenshot. Follow the PDF's S01-S24 checklist; add one short explanation below each image in a single PDF named with your own SRN, e.g. `YOUR_FULL_SRN.pdf`. Do not fabricate results or submit a separate report.

Example evidence commands, from this project directory:

```sh
pwd
gcc --version
make --version
ls -l
make clean && make
./campuscompute help
./campuscompute schedule compare jobs.txt
./campuscompute logger
cat campuscompute.log
./campuscompute ipc
./campuscompute banker safe.txt
./campuscompute banker unsafe.txt
./campuscompute worker normal
./campuscompute worker fail
./campuscompute limit
./campuscompute nonsense
./campuscompute demo
```

For S20, use the exact PIDs and IPC names printed by your own run: check that the child PIDs are gone (e.g. `ps -p PID`) and inspect the corresponding entries in `/dev/shm`; the program reports thread joins and cleanup. A `ps` query for an already-exited PID normally returns no matching process. Avoid claiming cleanup based only on a message if the actual check disagrees.

If the course portal requires a ZIP, create it only after adding your own screenshot PDF. Replace the SRN placeholder in this command with your actual PDF filename:

```sh
zip submission.zip *.c *.h Makefile README.md jobs.txt safe.txt unsafe.txt test.sh YOUR_FULL_SRN.pdf
unzip -l submission.zip
unzip -t submission.zip
```

Capture the ZIP listing and integrity check for S24, while following the course instructions. If S24 itself must appear inside the PDF included in the ZIP, capture it after a first ZIP, update the PDF with S24, and recreate the ZIP with the updated PDF; check the final archive again.
