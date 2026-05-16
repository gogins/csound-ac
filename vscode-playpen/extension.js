/**
 * Embed "playpen.py" in Visual Studio Code as the "Playpen" extension.
 * playpen.py, and playpen.ini, need to be in the user's home directory, or 
 * have a symbolic link in the user's home directory to the real file.
 */

const path = require('path');
const vscode = require('vscode');

function get_python_interpreter() {
	const configuration = vscode.workspace.getConfiguration('python');
	let python_interpreter = configuration.get('defaultInterpreterPath');
	if (!python_interpreter) {
		python_interpreter = 'python3';
	}
	vscode.window.setStatusBarMessage("Using python interpreter: " + python_interpreter);
	return python_interpreter;
}

/** Single-quoted POSIX shell escaping for paths that may contain spaces. */
function shSingleQuote(s) {
	return "'" + String(s).replace(/'/g, "'\\''") + "'";
}

/** Run terminal in the directory containing `filepath`; do not rely on workspace cwd. */
function spawner(shellCommandLine, filepath) {
	const cwd = path.dirname(filepath);
	let terminal_options = {"name": "Playpen",
		cwd};	
	let terminal = vscode.window.createTerminal(terminal_options);
	terminal.show(false);
	terminal.sendText(shellCommandLine, true);
}

function activatePythonPiece(uri) {
	let filepath = null;
	if (uri && uri.fsPath) {
		filepath = uri.fsPath;
	} else if (vscode.window.activeTextEditor) {
		filepath = vscode.window.activeTextEditor.document.uri.fsPath;
	}
	if (!filepath) {
		vscode.window.showErrorMessage('No Python file selected.');
		return;
	}
	if (!filepath.toLowerCase().endsWith('.py')) {
		vscode.window.showErrorMessage('Not a Python (.py) file: ' + filepath);
		return;
	}
	const python_interpreter = get_python_interpreter();
	let line;
	if (process.platform === 'win32') {
		line = '"' + python_interpreter.replace(/"/g, '\\"') + '" "' + filepath.replace(/"/g, '\\"') + '"';
	} else {
		line = `${shSingleQuote(python_interpreter)} ${shSingleQuote(filepath)}`;
	}
	spawner(line, filepath);
}

function activate(context) {
	console.log('Extension "playpen" is now active.');
	let disposable;
	disposable = vscode.commands.registerCommand('playpen.csd_audio', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		const python_interpreter = get_python_interpreter();
		spawner(`${python_interpreter} ~/playpen.py csd-audio ${filepath}`, filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.csd_soundfile', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		const python_interpreter = get_python_interpreter();
		spawner(`${python_interpreter} ~/playpen.py csd-play ${filepath}`, filepath);
	});	context.subscriptions.push(disposable);
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.csd_patch', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		const python_interpreter = get_python_interpreter();
		spawner(`${python_interpreter} ~/playpen.py csd-patch ${filepath}`, filepath);
	});	context.subscriptions.push(disposable);
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.html_localhost', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		const python_interpreter = get_python_interpreter();
		spawner(`${python_interpreter} ~/playpen.py html-localhost ${filepath}`, filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.html_nw', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		const python_interpreter = get_python_interpreter();
		spawner(`${python_interpreter} ~/playpen.py html-nw ${filepath}`, filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.cpp_lib', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		const python_interpreter = get_python_interpreter();
		spawner(`${python_interpreter} ~/playpen.py cpp-lib ${filepath}`, filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.cpp_app', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		const python_interpreter = get_python_interpreter();
		spawner(`${python_interpreter} ~/playpen.py cpp-app ${filepath}`, filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.cpp_audio', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		const python_interpreter = get_python_interpreter();
		spawner(`${python_interpreter} ~/playpen.py cpp-audio ${filepath}`, filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.cpp_soundfile', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		const python_interpreter = get_python_interpreter();
		spawner(`${python_interpreter} ~/playpen.py cpp-play ${filepath}`, filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.python_piece', function (uri) {
		activatePythonPiece(uri || null);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.html5Reference', function () {
		vscode.env.openExternal(vscode.Uri.parse("https://www.w3schools.com/jsref/default.asp"));
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.csoundReference', function () {
		vscode.env.openExternal(vscode.Uri.parse("https://csound.com/docs/manual/index.html"));		
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.csoundApiReference', function () {
		vscode.env.openExternal(vscode.Uri.parse("https://csound.com/docs/api/index.html"));		
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.csoundAcReference', function () {
		vscode.env.openExternal(vscode.Uri.parse("https://www.w3schools.com/jsref/default.asp"));
		
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.cppReference', function () {
		vscode.env.openExternal(vscode.Uri.parse("https://en.cppreference.com/w/"));
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.pythonReference', function () {
		vscode.env.openExternal(vscode.Uri.parse("https://docs.python.org/3/"));
	});
	context.subscriptions.push(disposable);
}

// This method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
