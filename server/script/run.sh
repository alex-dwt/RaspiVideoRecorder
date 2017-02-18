#! /bin/bash
# This file is part of the RaspiVideoRecorder package.
# (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
# For the full copyright and license information, please view the LICENSE file that was distributed with this source code.

set -e

service cron start
echo '*/3 * * * * /bin/bash -c "/recorder/script/cleaner.sh"' | crontab -

npm start