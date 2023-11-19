/**
 * Embed "playpen.py" in Visual Studio Code as the "Playpen" extension.
 * playpen.py, and playpen.ini, need to be in the user's home directory, or 
 * have a symbolic link in the user's home directory to the real file.
 */

const path = require('node:path');
const vscode = require('vscode');
const child_process = require('child_process');
let process_map = {};
let output_channel;

/**
 * Execute the command string in the directory of the document.
 */
async function spawner(command, filepath) {
	const options = { cwd: path.dirname(filepath), shell: true };
	if (output_channel == undefined) {
		output_channel = vscode.window.createOutputChannel('Playpen');
	};
    output_channel.clear();
    output_channel.show(true); // true means keep focus in the editor window
	let args = command.split(' ');
	let cmd = args[0];
	let params = args.slice(1);

    const process = child_process.spawn(cmd, params, options);
    process_map[process.pid] = process;
    process.stdout.on("data", (data) => {
        // I've seen spurious 'ANSI reset color' sequences in some csound output
        // which doesn't render correctly in this context. Stripping that out here.
        output_channel.append(data.toString().replace(/\x1b\[m/g, ""));
    });
    process.stderr.on("data", (data) => {
        // It looks like all csound output is written to stderr, actually.
        // If you want your changes to show up, change this one.
        output_channel.append(data.toString().replace(/\x1b\[m/g, ""));
    });
    if (process.pid) {
        output_channel.append("Playpen is running (pid " + process.pid + ")");
    }
}

function activate(context) {
	console.log('Extension "playpen" is now active.');
	let disposable;
	disposable = vscode.commands.registerCommand('playpen.renderCsdToAudio', function () {
		let filepath = vscode.window.activeTextEditor.document.uri.fsPath;
		console.log(`Rendering "${filepath}..."`)
		spawner(`python3 ~/playpen.py csd-audio ${filepath}`, filepath);
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
