/**
 * Embed "playpen.py" in Visual Studio Code as the "Playpen" extension.
 * playpen.py, and playpen.ini, need to be in the user's home directory, or 
 * have a symbolic link in the user's home directory to the real file.
 */

const fs = require('fs');
const os = require('os');
const path = require('path');
const vscode = require('vscode');

function read_playpen_ini_python_interpreter() {
	const iniPath = path.join(os.homedir(), 'playpen.ini');
	try {
		const text = fs.readFileSync(iniPath, 'utf8');
		let section = '';
		for (const rawLine of text.split(/\r?\n/)) {
			const line = rawLine.trim();
			if (!line || line.startsWith('#') || line.startsWith(';')) {
				continue;
			}
			const sectionMatch = line.match(/^\[([^\]]+)\]$/);
			if (sectionMatch) {
				section = sectionMatch[1].trim().toLowerCase();
				continue;
			}
			const keyValue = line.match(/^([^=]+?)\s*=\s*(.*)$/);
			if (section === 'playpen' && keyValue && keyValue[1].trim() === 'python-interpreter') {
				return keyValue[2].trim();
			}
		}
	} catch (_error) {
		// Fall back to Cursor's Python setting below.
	}
	return '';
}

function get_python_interpreter() {
	const ini_interpreter = read_playpen_ini_python_interpreter();
	if (ini_interpreter) {
		vscode.window.setStatusBarMessage("Using python interpreter from ~/playpen.ini: " + ini_interpreter);
		return ini_interpreter;
	}
	const configuration = vscode.workspace.getConfiguration('python');
	let python_interpreter = configuration.get('defaultInterpreterPath');
	const defaultVenv = path.join(os.homedir(), 'venv', 'csound', 'bin', 'python');
	if (!python_interpreter) {
		python_interpreter = fs.existsSync(defaultVenv) ? defaultVenv : 'python3';
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

function quoteForShell(s) {
	if (process.platform === 'win32') {
		return '"' + String(s).replace(/"/g, '\\"') + '"';
	}
	return shSingleQuote(s);
}

function playpenCommand(subcommand, filepath) {
	const python_interpreter = get_python_interpreter();
	const playpen_script = path.join(os.homedir(), 'playpen.py');
	return `${quoteForShell(python_interpreter)} ${quoteForShell(playpen_script)} ${subcommand} ${quoteForShell(filepath)}`;
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
	const line = `${quoteForShell(python_interpreter)} ${quoteForShell(filepath)}`;
	spawner(line, filepath);
}

function activate(context) {
	console.log('Extension "playpen" is now active.');
	let disposable;
	disposable = vscode.commands.registerCommand('playpen.csd_audio', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(playpenCommand('csd-audio', filepath), filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.csd_soundfile', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(playpenCommand('csd-play', filepath), filepath);
	});	context.subscriptions.push(disposable);
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.csd_patch', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(playpenCommand('csd-patch', filepath), filepath);
	});	context.subscriptions.push(disposable);
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.html_localhost', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(playpenCommand('html-localhost', filepath), filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.html_nw', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(playpenCommand('html-nw', filepath), filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.cpp_lib', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(playpenCommand('cpp-lib', filepath), filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.cpp_app', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(playpenCommand('cpp-app', filepath), filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.cpp_audio', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(playpenCommand('cpp-audio', filepath), filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.cpp_soundfile', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(playpenCommand('cpp-play', filepath), filepath);
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
