/*
 * This file is part of the RaspiVideoRecorder package.
 * (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
 * For the full copyright and license information, please view the LICENSE file that was distributed with this source code.
 */

import {execSync, spawn} from 'child_process';

const PATH = '/recorder/script';
const FILE = 'recorder.sh';

export default class {
    static start() {
        if (this.isWorking()) {
            return;
        }

        let proc = spawn(
            `${PATH}/${FILE}`,
            [ ],
            {detached: true, stdio: ['ignore', 'ignore', 'ignore']}
        );
        proc.unref();
    }

    static stop() {
        execSync(`pkill -SIGINT ${FILE}; exit 0`);
    }

    static isWorking() {
        return ! parseInt(execSync(`pgrep ${FILE} > /dev/null 2>&1; echo $?`).toString());
    }
}
