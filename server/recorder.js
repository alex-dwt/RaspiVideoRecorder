/*
 * This file is part of the RaspiVideoRecorder package.
 * (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
 * For the full copyright and license information, please view the LICENSE file that was distributed with this source code.
 */

import {execSync, spawn} from 'child_process';

const RECORDER_BIN = 'v4l2-ctl';
const REFRESH_TIMEOUT = 15;

let refreshTimeout;
let proc;
// let currentDir = '';
let currentFile = '';
let currentFps = '';

export default class {
    static start() {
        if (this.isWorking()) {
            return;
        }

        updateStatus();

        proc = spawn(
            RECORDER_BIN,
            ['-d', process.env.VIDEO_DEV, '--stream-mmap=3', '--stream-to=/dummy']
        );
        proc.stderr.setEncoding('utf8');
        proc.stderr.on('data', (data) => updateStatus(data));
        proc.on('close', () => updateStatus());
    }

    static stop() {
        updateStatus();
    }

    static isWorking() {
        return ! parseInt(execSync(`pgrep ${RECORDER_BIN} > /dev/null 2>&1; echo $?`).toString());
    }

    static getInfo() {
        let file = currentFile;
        {
            let pos = file.lastIndexOf('_');
            let currentFile = pos !== -1
                ? file.substring(0, pos)
                : file;

            return {
                isWorking: this.isWorking(),
                // currentDir,
                currentFile,
                currentFps
            };
        }
    }
}

function updateStatus(data) {
    clearTimeout(refreshTimeout);
    refreshTimeout = setTimeout(
        () => updateStatus(),
        REFRESH_TIMEOUT * 1000
    );

    if (typeof data !== 'undefined') {
        let matches = data.match(/^(##(.+?)##(.+?)##(.+?)##\s)+$/);
        if (matches && currentFile != matches[3]) {
            // currentDir = matches[2];
            currentFile = matches[3];
            currentFps = matches[4];

            return;
        }
    }

    // wrong data was sent from recorder
    // or currentFile and previousFile is equal
    // or timeout was triggered
    execSync(`pkill -SIGINT ${RECORDER_BIN}; exit 0`);
    // currentDir = '';
    currentFile = '';
    currentFps = '';
}

