/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */
import {
	createConnection,
	TextDocuments,
	Diagnostic,
	DiagnosticSeverity,
	ProposedFeatures,
	InitializeParams,
	DidChangeConfigurationNotification,
	CompletionItem,
	CompletionItemKind,
	TextDocumentPositionParams,
	TextDocumentSyncKind,
	InitializeResult,
	DocumentDiagnosticReportKind,
	type DocumentDiagnosticReport,
	Hover
} from 'vscode-languageserver/node';

import {
	TextDocument
} from 'vscode-languageserver-textdocument';
import { XMLParser } from 'fast-xml-parser';

// Create a connection for the server, using Node's IPC as a transport.
// Also include all preview / proposed LSP features.
const connection = createConnection(ProposedFeatures.all);

// Create a simple text document manager.
const documents = new TextDocuments(TextDocument);

let hasConfigurationCapability = false;
let hasWorkspaceFolderCapability = false;
let hasDiagnosticRelatedInformationCapability = false;

const parser = new XMLParser({
	ignoreAttributes: false,
	attributeNamePrefix: '',
	allowBooleanAttributes: true
});

connection.onInitialize((params: InitializeParams) => {
	const capabilities = params.capabilities;

	// Does the client support the `workspace/configuration` request?
	// If not, we fall back using global settings.
	hasConfigurationCapability = !!(
		capabilities.workspace && !!capabilities.workspace.configuration
	);
	hasWorkspaceFolderCapability = !!(
		capabilities.workspace && !!capabilities.workspace.workspaceFolders
	);
	hasDiagnosticRelatedInformationCapability = !!(
		capabilities.textDocument &&
		capabilities.textDocument.publishDiagnostics &&
		capabilities.textDocument.publishDiagnostics.relatedInformation
	);

	const result: InitializeResult = {
		capabilities: {
			textDocumentSync: TextDocumentSyncKind.Incremental,
			// Tell the client that this server supports code completion.
			completionProvider: {
				resolveProvider: false
			},
			hoverProvider: true
		}
	};
	if (hasWorkspaceFolderCapability) {
		result.capabilities.workspace = {
			workspaceFolders: {
				supported: true
			}
		};
	}
	return result;
});

connection.onInitialized(() => {
	if (hasConfigurationCapability) {
		// Register for all configuration changes.
		connection.client.register(DidChangeConfigurationNotification.type, undefined);
	}
	if (hasWorkspaceFolderCapability) {
		connection.workspace.onDidChangeWorkspaceFolders(_event => {
			connection.console.log('Workspace folder change event received.');
		});
	}
});

// The example settings
interface ExampleSettings {
	maxNumberOfProblems: number;
}

// The global settings, used when the `workspace/configuration` request is not supported by the client.
// Please note that this is not the case when using this server with the client provided in this example
// but could happen with other clients.
const defaultSettings: ExampleSettings = { maxNumberOfProblems: 1000 };
let globalSettings: ExampleSettings = defaultSettings;

// Cache the settings of all open documents
const documentSettings = new Map<string, Thenable<ExampleSettings>>();

connection.onDidChangeConfiguration(change => {
	if (hasConfigurationCapability) {
		// Reset all cached document settings
		documentSettings.clear();
	} else {
		globalSettings = (
			(change.settings.languageServerExample || defaultSettings)
		);
	}
	// Refresh the diagnostics since the `maxNumberOfProblems` could have changed.
	// We could optimize things here and re-fetch the setting first can compare it
	// to the existing setting, but this is out of scope for this example.
	connection.languages.diagnostics.refresh();
});

function getDocumentSettings(resource: string): Thenable<ExampleSettings> {
	if (!hasConfigurationCapability) {
		return Promise.resolve(globalSettings);
	}
	let result = documentSettings.get(resource);
	if (!result) {
		result = connection.workspace.getConfiguration({
			scopeUri: resource,
			section: 'languageServerExample'
		});
		documentSettings.set(resource, result);
	}
	return result;
}

// Only keep settings for open documents
documents.onDidClose(e => {
	documentSettings.delete(e.document.uri);
});

/*
connection.languages.diagnostics.on(async (params) => {
	const document = documents.get(params.textDocument.uri);
	if (document !== undefined) {
		return {
			kind: DocumentDiagnosticReportKind.Full,
			items: await validateTextDocument(document)
		} satisfies DocumentDiagnosticReport;
	} else {
		// We don't know the document. We can either try to read it from disk
		// or we don't report problems for it.
		return {
			kind: DocumentDiagnosticReportKind.Full,
			items: []
		} satisfies DocumentDiagnosticReport;
	}
}); */

// The content of a text document has changed. This event is emitted
// when the text document first opened or when its content has changed.
documents.onDidChangeContent(change => {
	validateTextDocument(change.document);
});

async function validateTextDocument(textDocument: TextDocument): Promise<Diagnostic[] | undefined> {
	// In this simple example we get the settings for every validate run.
	const settings = await getDocumentSettings(textDocument.uri);

	const text = textDocument.getText();
	const diagnostics: Diagnostic[] = [];

	let parsed: any;

	try {
		parsed = parser.parse(text);
	} catch (e) {
		diagnostics.push(makeDiag('Invalid XML structure', DiagnosticSeverity.Error));
		send(diagnostics);
		return;
	}

	if (!parsed.osm) {
		diagnostics.push(makeDiag('Root element must be <osm>', DiagnosticSeverity.Error));
		send(diagnostics);
		return;
	}

	const osm = parsed.osm;

	const nodeIds = new Set<string>();
	const wayIds = new Set<string>();
	const relationIds = new Set<string>();

	const nodes = toArray(osm.node);
	const ways = toArray(osm.way);
	const relations = toArray(osm.relation);

	nodes.forEach(n => n.id && nodeIds.add(String(n.id)));
	ways.forEach(w => w.id && wayIds.add(String(w.id)));
	relations.forEach(r => r.id && relationIds.add(String(r.id)));

	nodes.forEach(n => {
		if (n.lat === undefined || n.lon === undefined) {
		diagnostics.push(makeDiag('<node> must have lat and lon', DiagnosticSeverity.Error));
		} else {
			const lat = Number(n.lat);
			const lon = Number(n.lon);

			if (lat < -90 || lat > 90) {
				diagnostics.push(makeDiag(`Invalid latitude: ${lat}`, DiagnosticSeverity.Error));
			}
			if (lon < -180 || lon > 180) {
				diagnostics.push(makeDiag(`Invalid longitude: ${lon}`, DiagnosticSeverity.Error));
			}
		}

		validateTags(n.tag, diagnostics);
	});

	ways.forEach(w => {
		const nds = toArray(w.nd);

		if (nds.length < 2) {
		diagnostics.push(makeDiag('<way> must have at least 2 <nd> refs', DiagnosticSeverity.Warning));
		}

		nds.forEach(nd => {
		if (!nd.ref) {
			diagnostics.push(makeDiag('<nd> must have ref', DiagnosticSeverity.Error));
		} else if (!nodeIds.has(String(nd.ref))) {
			diagnostics.push(makeDiag(`nd ref ${nd.ref} does not exist`, DiagnosticSeverity.Error));
		}
		});

		validateTags(w.tag, diagnostics);
	});

	relations.forEach(r => {
		const members = toArray(r.member);

		members.forEach(m => {
		if (!m.type || !m.ref) {
			diagnostics.push(makeDiag('<member> must have type and ref', DiagnosticSeverity.Error));
			return;
		}

		const ref = String(m.ref);

		if (m.type === 'node' && !nodeIds.has(ref)) {
			diagnostics.push(makeDiag(`member node ref ${ref} missing`, DiagnosticSeverity.Error));
		}
		if (m.type === 'way' && !wayIds.has(ref)) {
			diagnostics.push(makeDiag(`member way ref ${ref} missing`, DiagnosticSeverity.Error));
		}
		if (m.type === 'relation' && !relationIds.has(ref)) {
			diagnostics.push(makeDiag(`member relation ref ${ref} missing`, DiagnosticSeverity.Error));
		}
		});

		validateTags(r.tag, diagnostics);
	});

	send(diagnostics);

	function send(diags: Diagnostic[]) {
		connection.sendDiagnostics({ uri: textDocument.uri, diagnostics: diags });
	}
}

connection.onDidChangeWatchedFiles(_change => {
	// Monitored files have change in VSCode
	connection.console.log('We received a file change event');
});

// This handler provides the initial list of the completion items.
connection.onCompletion((): CompletionItem[] => {
	return [
		...tags.map(k => ({
			label: k,
			kind: CompletionItemKind.Property
		}))
	];
});

// This handler resolves additional information for the item selected in
// the completion list.
connection.onCompletionResolve(
	(item: CompletionItem): CompletionItem => {
		if (item.data === 1) {
			item.detail = 'TypeScript details';
			item.documentation = 'TypeScript documentation';
		} else if (item.data === 2) {
			item.detail = 'JavaScript details';
			item.documentation = 'JavaScript documentation';
		}
		return item;
	}
);

function toArray(obj: any): any[] {
	if (!obj) return [];
	return Array.isArray(obj) ? obj : [obj];
}

function makeDiag(message: string, severity: DiagnosticSeverity): Diagnostic {
	return {
		severity,
		range: {
			start: { line: 0, character: 0 },
			end: { line: 0, character: 1 }
		},
		message,
		source: 'osm-lsp'
	}
}

function validateTags(tags: any, diagnostics: Diagnostic[]) {
  	const arr = toArray(tags);
  	arr.forEach(t => {
   		if (!t.k || !t.v) {
   	   		diagnostics.push(makeDiag('<tag> must have k and v', DiagnosticSeverity.Warning));
   		}
 	});
}

const tags = ['bounds', 'node', 'way', 'relation'];

connection.onHover((): Hover => {
  return {
    contents: {
      kind: 'markdown',
      value: `**OSM Tag**\n\nExample: \`<tag k="highway" v="residential"/>\``
    }
  };
});

// Make the text document manager listen on the connection
// for open, change and close text document events
documents.listen(connection);

// Listen on the connection
connection.listen();