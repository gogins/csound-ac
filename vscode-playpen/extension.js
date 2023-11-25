/**
 * Embed "playpen.py" in Visual Studio Code as the "Playpen" extension.
 * playpen.py, and playpen.ini, need to be in the user's home directory, or 
 * have a symbolic link in the user's home directory to the real file.
 */

const path = require('path');
const vscode = require('vscode');

/**
 * Execute the command text in the directory of the document.
 * A Terminal is used rather than a subprocess with an output channel, 
 * because it is simpler, and only a Terminal can kill all subprocesses when 
 * the user kills the Terminal. The cwd is set to the directory containing 
 * the filepath.
 */
async function spawner(command, filepath) {
	let terminal_options = {"name": "Playpen",
		"cwd": path.dirname(filepath)};	
	let terminal = vscode.window.createTerminal(terminal_options);
	terminal.show(false);
	terminal.sendText("cd "  + path.dirname(filepath), true);
	terminal.sendText(command, true);
}

function activate(context) {
	console.log('Extension "playpen" is now active.');
	let disposable;
	disposable = vscode.commands.registerCommand('playpen.csd_audio', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(`python3 ~/playpen.py csd-audio ${filepath}`, filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.csd_soundfile', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(`python3 ~/playpen.py csd-play ${filepath}`, filepath);
	});	context.subscriptions.push(disposable);
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.csd_psyvh', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(`python3 ~/playpen.py csd-patch ${filepath}`, filepath);
	});	context.subscriptions.push(disposable);
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.html_localhost', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(`python3 ~/playpen.py html-localhost ${filepath}`, filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.html_nw', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(`python3 ~/playpen.py html-nw ${filepath}`, filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.cpp_lib', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(`python3 ~/playpen.py cpp-lib ${filepath}`, filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.cpp_app', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(`python3 ~/playpen.py cpp-app ${filepath}`, filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.cpp_audio', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(`python3 ~/playpen.py cpp-audio ${filepath}`, filepath);
	});
	context.subscriptions.push(disposable);
	disposable = vscode.commands.registerCommand('playpen.cpp_soundfile, function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(`python3 ~/playpen.py cpp-play ${filepath}`, filepath);
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
