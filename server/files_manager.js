/*
 * This file is part of the RaspiVideoRecorder package.
 * (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
 * For the full copyright and license information, please view the LICENSE file that was distributed with this source code.
 */

import {execSync} from 'child_process';

const FILES_PATH = '/tmp/images';

export default class {
    static getDirs() {
        let parse = dirs => {
            dirs = dirs.split('\n');
            dirs.pop();
            return dirs.map(item => ({
                dirName: item,
                name: new Date(item.replace('_', '') * 1000).toISOString().substr(0,19).replace('T', ' ')
            }));
        };

        return {
            saved: parse(execSync(`ls ${FILES_PATH} | grep ^_ | sort -r`).toString()),
            current: parse(execSync(`ls ${FILES_PATH} | grep -v ^_ | sort -r`).toString())
        };
    }

    static saveDir(dirName) {
        dirName = parseInt(dirName);
        if (isNaN(dirName)) {
            return;
        }
        if (this.isWorking()) {
            let lastDirName = ''; // todo
            if (lastDirName === dirName) {
                // we cannot move directory which is being used by recorder right now
                return;
            }
        }

        execSync(`cd ${FILES_PATH} ; mv ${dirName} _${dirName} ; exit 0`)
    }
}