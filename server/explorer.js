/*
 * This file is part of the RaspiVideoRecorder package.
 * (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
 * For the full copyright and license information, please view the LICENSE file that was distributed with this source code.
 */

import {execSync} from 'child_process';
import Converter from './converter'

const FILES_PATH = '/RecordedData/Images';
const TMP_IMAGES_PATH = '/RecordedData/Tmpfs';

export default class {
    static getDirs() {
        let parse = (dirs, checkCanBeDeleted, checkCanBeSaved) => {
            dirs = dirs.split('\n');
            dirs.pop();

            return dirs.map(item => (Object.assign(
                {
                    dirName: item,
                    name: new Date(item.replace('_', '') * 1000).toISOString().substr(0,19).replace('T', ' ')
                },
                checkCanBeSaved ? {canBeSaved: isDirCanBeSaved(item)} : {},
                checkCanBeDeleted ? {canBeDeleted: isDirCanBeDeleted(item)} : {}
            )));
        };

        return {
            saved: parse(
                execSync(`ls -r ${FILES_PATH} 2>/dev/null | grep "^_[0-9]\\+$" ; exit 0`).toString(),
                true
            ),
            current: parse(
                execSync(`ls -r ${FILES_PATH} 2>/dev/null | grep "^[0-9]\\+$" ; exit 0`).toString(),
                false,
                true
            )
        };
    }

    static saveDir(dirName) {
        dirName = parseInt(dirName);
        if (isNaN(dirName)) {
            return false;
        }

        if (!isDirCanBeSaved(dirName)) {
            return false;
        }

        execSync(`cd ${FILES_PATH} ; mv ${dirName} _${dirName} ; touch _${dirName}/ready ; exit 0`);

        return true;
    }

    static deleteDir(dirName) {
        if (!isDirCanBeDeleted(dirName)) {
            return false;
        }

        execSync(`cd ${FILES_PATH} ; rm -rf ${dirName} ; exit 0`);

        return true;
    }
}

/**
 * We can remove saved dirs only if "ConverterDaemon" is off
 */
function isDirCanBeDeleted(dirName) {
    return !Converter.isWorking();
}

/**
 * If directory with this name has been already removed from Tmpfs directory
 * it means that "Recorder" is not using it anymore and therefore we can remove it
 */
function isDirCanBeSaved(dirName) {
    dirName = parseInt(dirName);
    if (isNaN(dirName)) {
        return false;
    }

    return execSync(`ls ${TMP_IMAGES_PATH} | grep ${dirName} | wc -l ; exit 0`).toString() == 0;
}
