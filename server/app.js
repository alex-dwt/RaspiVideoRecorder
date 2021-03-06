/*
 * This file is part of the RaspiVideoRecorder package.
 * (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
 * For the full copyright and license information, please view the LICENSE file that was distributed with this source code.
 */

'use strict';

import express from 'express';
import HttpException from './http_exception'
import Recorder from './recorder'
import Converter from './converter'
import Explorer from './explorer'

let app = express();

app.use(function(req, res, next) {
	res.header('Access-Control-Allow-Origin', '*');
	res.header('Access-Control-Allow-Headers', 'Origin, X-Requested-With, Content-Type, Accept');
	res.header('Access-Control-Allow-Methods', 'POST, GET, PUT, DELETE, OPTIONS');

	if (req.method.toLowerCase() === 'options') {
		res.status(204).send();
	} else {
		next();
	}
});

/**
 * Files explorer
 */
// mark dir as saved
app.put('/explorer/:dir', (req, res, next) => {
    res.json({
    	success: Explorer.saveDir(req.params.dir)
    });
});
// remove dir
app.delete('/explorer/:dir', (req, res, next) => {
    res.json({
    	success: Explorer.deleteDir(req.params.dir)
    });
});
// list with images directories
app.get('/explorer', (req, res, next) => {
    res.json(Explorer.getDirs());
});

/**
 * Recorder
 */
// status
// app.get('/recorder', (req, res, next) => {
//     res.json(Recorder.getInfo());
// });
// start
app.put('/recorder', (req, res, next) => {
	Recorder.start();
	res.json({ success: true });
});
// stop
app.delete('/recorder', (req, res, next) => {
    Recorder.stop();
    res.json({ success: true });
});

/**
 * Converter
 */
// status
// app.get('/converter', (req, res, next) => {
//     res.json(Converter.getInfo());
// });
// start
app.put('/converter', (req, res, next) => {
    Converter.start();
	res.json({ success: true });
});
// stop
app.delete('/converter', (req, res, next) => {
    Converter.stop();
	res.json({ success: true });
});

/**
 * Converter & Recorder statuses
 */
// status
app.get('/status', (req, res, next) => {
    res.json({
    	converter: Converter.getInfo(),
		recorder: Recorder.getInfo()
	});
});

/**
 * Errors handlers
 */
app.use((req, res, next) => {
    next(new HttpException(404, 'Url does not exist'));
});

app.use((err, req, res, next) => {
	if (err instanceof HttpException) {
		res.status(err.code).json(err);
	} else {
		console.error(err.stack);
		res.status(500).end();
	}
});

app.listen(80);
