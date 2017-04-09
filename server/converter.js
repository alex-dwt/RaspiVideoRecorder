/*
 * This file is part of the RaspiVideoRecorder package.
 * (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
 * For the full copyright and license information, please view the LICENSE file that was distributed with this source code.
 */

import {execSync, spawn} from 'child_process';

const CONVERTER_DAEMON_SCRIPT = '/recorder/script/converter_daemon.sh';
const REFRESH_TIMEOUT = 30;

let refreshTimeout;
let proc;
let currentFile = '';
let currentProgress = '';

export default class {
    static start() {
        if (this.isWorking()) {
            return;
        }

        updateStatus();

        proc = spawn(
            '/bin/bash',
            ['-c', CONVERTER_DAEMON_SCRIPT]
        );
        proc.stdout.setEncoding('utf8');
        proc.stdout.on('data', (data) => updateStatus(data));
        proc.on('close', () => updateStatus());
    }

    static stop() {
        updateStatus();
    }

    static isWorking() {
        return ! parseInt(execSync(`pgrep -f "${CONVERTER_DAEMON_SCRIPT}$" > /dev/null 2>&1; echo $?`).toString());
    }

    static getInfo() {
        return {
            isWorking: this.isWorking(),
            currentFile,
            currentProgress
        };
    }
}

function updateStatus(data) {
    clearTimeout(refreshTimeout);
    refreshTimeout = setTimeout(
        () => updateStatus(),
        REFRESH_TIMEOUT * 1000
    );

    if (typeof data !== 'undefined') {
        // from script we can get lines like this
        // - "100%" : current progress
        // - "@-" : current processed file
        let matches = data.match(/^@(.*?)\s$/);
        if (matches) {
            currentFile = matches[1];
            if (currentFile === '') {
                currentProgress = '';
            }
            return;
        }

        matches = data.match(/^(\d+%)\s$/);
        if (matches) {
            currentProgress = matches[1];
            return;
        }
    }

    // wrong data was sent from recorder
    // or timeout was triggered
    execSync(`pkill -f ${CONVERTER_DAEMON_SCRIPT}; exit 0`);
    currentFile = '';
    currentProgress = '';
}

