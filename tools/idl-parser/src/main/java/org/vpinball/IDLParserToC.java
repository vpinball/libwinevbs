package org.vpinball;

import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.vpinball.Param.ParamType;

public class IDLParserToC {

	 private static class ParsedInterface {
		String name;
		String parent;
		List<Method> methods = new ArrayList<Method>();
	 }

	 public void parse(String in, String out, List<IDLInterface> interfaceList) throws Exception {
		HashMap<String, IDLInterface> interfaceMap = new HashMap<String, IDLInterface>();
		for (IDLInterface idlClass:interfaceList) {
			interfaceMap.put(idlClass.getInterfaceName(), idlClass);
		}

		Map<String, ParsedInterface> all = readInterfaces(in);

		FileOutputStream outputStream = new FileOutputStream(out);

		outputStream.write("#include \"libwinevbs.h\"\n".getBytes());
		outputStream.write("\n".getBytes());

		for (String name : all.keySet()) {
			if (!interfaceMap.containsKey(name)) {
				continue;
			}
			IDLInterface idlInterface = interfaceMap.get(name);
			LinkedHashMap<String, Dispatch> dispatchMap = flatten(name, all);
			outputStream.write(generateDispatch(idlInterface, dispatchMap, interfaceMap).getBytes());
		}

		outputStream.close();
	 }

	 private Map<String, ParsedInterface> readInterfaces(String in) throws Exception {
		Map<String, ParsedInterface> result = new LinkedHashMap<String, ParsedInterface>();

		BufferedReader bufferedReader = new BufferedReader(new FileReader(in));

		int lineNo = 0;
		ParsedInterface current = null;
		boolean foundMethod = false;
		int methodLineNo = 0;
		String methodBuffer = "";

		String line = bufferedReader.readLine();

		while (line != null) {
			 lineNo++;
			 line = line.trim();

			 if (current == null) {
				if (line.startsWith("interface ") && line.contains(":")) {
					 String[] parts = line.substring("interface ".length()).split(":", 2);
					 String name = parts[0].trim();
					 String parentTok = parts[1].trim();
					 String[] parentParts = parentTok.split("[\\s{]");
					 String parent = parentParts[0].trim();

					 current = new ParsedInterface();
					 current.name = name;
					 current.parent = parent;
					 result.put(name, current);

					 foundMethod = false;
					 methodBuffer = "";
				}
			 }
			 else if (line.startsWith("}")) {
				current = null;
			 }
			 else {
				if (!foundMethod && line.startsWith("[")) {
					 foundMethod = true;
					 methodLineNo = lineNo;
				}

				if (foundMethod) {
					 methodBuffer += line;

					 if (line.endsWith(";")) {
						Method method = new Method(methodBuffer, methodLineNo);
						current.methods.add(method);

						foundMethod = false;
						methodBuffer = "";
					 }
				}
			 }

			 line = bufferedReader.readLine();
		}

		bufferedReader.close();

		return result;
	 }

	 private LinkedHashMap<String, Dispatch> flatten(String name, Map<String, ParsedInterface> all) {
		List<String> chain = new ArrayList<String>();
		String cursor = name;
		while (cursor != null && all.containsKey(cursor)) {
			 chain.add(cursor);
			 cursor = all.get(cursor).parent;
		}

		LinkedHashMap<String, Dispatch> dispatchMap = new LinkedHashMap<String, Dispatch>();
		for (int i = chain.size() - 1; i >= 0; i--) {
			 ParsedInterface pi = all.get(chain.get(i));
			 for (Method method : pi.methods) {
				if (dispatchMap.containsKey(method.getName())) {
					 dispatchMap.get(method.getName()).addMethod(method);
				}
				else {
					 Dispatch dispatch = new Dispatch();
					 dispatch.setId(method.getId());
					 dispatch.addMethod(method);
					 dispatchMap.put(method.getName(), dispatch);
				}
			 }
		}
		return dispatchMap;
	 }

	 private static boolean isDispatchDerivedRetvalType(String type) {
		return "IFontDisp**".equals(type)
				|| "ITable**".equals(type)
				|| "IDrive**".equals(type)
				|| "IDriveCollection**".equals(type)
				|| "IFolderCollection**".equals(type)
				|| "IFileCollection**".equals(type)
				|| "IFile**".equals(type)
				|| "IFolder**".equals(type)
				|| "ITextStream**".equals(type)
				|| "IWshCollection**".equals(type)
				|| "IWshEnvironment**".equals(type)
				|| "IWshExec**".equals(type)
				|| "IDispatch**".equals(type);
	 }

	 private static boolean isChainablePropget(Method method) {
		if (method.getType() != Method.InvokeType.PROPERTYGET) {
			return false;
		}
		if (method.getCArgs() != 0) {
			return false;
		}
		for (Param p : method.getParamList()) {
			if (p.getParamType() == ParamType.OUT_RETVAL) {
				return isDispatchDerivedRetvalType(p.getType());
			}
		}
		return false;
	 }

	 private String generateDispatch(IDLInterface idlInterface, LinkedHashMap<String, Dispatch> dispatchMap, HashMap<String, IDLInterface> interfaceMap) throws Exception {
		StringBuffer buffer = new StringBuffer();

		buffer.append("static HRESULT WINAPI " + idlInterface.getClassName() + "_GetIDsOfNames(" + idlInterface.getInterfaceName() + " *iface, REFIID riid, LPOLESTR *rgszNames,\n");
		buffer.append("\t\tUINT cNames, LCID lcid, DISPID *rgDispId)\n");
		buffer.append("{\n");

		StringBuffer fragment = new StringBuffer();

		fragment.append("static struct {\n");
		fragment.append("const WCHAR *name;\n");
		fragment.append("DISPID dispId;\n");
		fragment.append("} names_ids_list[] = {\n");
		fragment.append("{ NULL }");

		ArrayList<String> keys = new ArrayList<>(dispatchMap.keySet());
		Collections.sort(keys, String.CASE_INSENSITIVE_ORDER);

		boolean hasDispIdValue = false;

		for (String key : keys) {
			if (dispatchMap.containsKey(key)) {
				Dispatch dispatch = dispatchMap.get(key);

				if (dispatch.isDispIdNewEnum()) {
					continue;
				}

				if (dispatch.isDispIdValue()) {
					hasDispIdValue = true;
				}

				fragment.append(",\n");
				fragment.append("{ L\"" + key + "\", " + dispatch.getId() + " }");
			}
			else {
				System.out.println("\"" + key + "\" not found");
			}
		}

		fragment.append("\n");
		fragment.append("};\n");
		fragment.append("\n");

		fragment.append("size_t min = 1, max = ARRAY_SIZE(names_ids_list) - 1, i;\n");
		fragment.append("int r;\n");
		fragment.append("while(min <= max) {\n");
		fragment.append("i = (min + max) / 2;\n");
		fragment.append("r = wcsicmp(names_ids_list[i].name, *rgszNames);\n");
		fragment.append("if (!r) {\n");
		fragment.append("*rgDispId = names_ids_list[i].dispId;\n");
		fragment.append("return S_OK;\n");
		fragment.append("}\n");
		fragment.append("if (r < 0)\n");
		fragment.append("\tmin = i+1;\n");
		fragment.append("else\n");
		fragment.append("\tmax = i-1;\n");
		fragment.append("}\n");
		fragment.append("return DISP_E_MEMBERNOTFOUND;\n");

		buffer.append(indent(fragment.toString(), 1));
		buffer.append("}\n\n");

		buffer.append("static HRESULT WINAPI " + idlInterface.getClassName() + "_Invoke(" + idlInterface.getInterfaceName() + " *iface, DISPID dispIdMember,\n"
				+ "\t\tREFIID riid, LCID lcid, WORD wFlags,\n"
				+ "\t\tDISPPARAMS *pDispParams, VARIANT *pVarResult,\n"
				+ "\t\tEXCEPINFO *pExcepInfo, UINT *puArgErr)\n"
				+ "{\n");

		fragment.setLength(0);

		fragment.append("int index = pDispParams->cArgs;\n");
		fragment.append("VARIANT res;\n");
		fragment.append("HRESULT hres = DISP_E_UNKNOWNNAME;\n");
		fragment.append("\n");
		fragment.append("V_VT(&res) = VT_EMPTY;\n");
		fragment.append("\n");
		fragment.append("switch(dispIdMember) {\n");

		if (!hasDispIdValue) {
			fragment.append("case DISPID_VALUE: {\n");
			fragment.append("if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {\n");
			fragment.append("// Default method\n");
			fragment.append("V_VT(&res) = VT_DISPATCH;\n");
			fragment.append("V_DISPATCH(&res) = (IDispatch*)iface;\n");
			fragment.append("hres = S_OK;\n");
			fragment.append("}\n");
			fragment.append("break;\n");
			fragment.append("}\n");
		}

		int index = 0;

		for (String key : dispatchMap.keySet()) {
			 Dispatch dispatch = dispatchMap.get(key);

			 fragment.append("case " + dispatch.getId() + ": {\n");

			 index = 0;

			 for (Method method:dispatch.getMethodList()) {
				if (index > 0) {
					 fragment.append("else ");
				}
				fragment.append(generateMethod(method, idlInterface.getClassName()));
				index++;
			 }

				if (dispatch.isDispIdValue()) {
				fragment.append("else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {\n");
				fragment.append("// Default method\n");
				fragment.append("V_VT(&res) = VT_DISPATCH;\n");
				fragment.append("V_DISPATCH(&res) = (IDispatch*)iface;\n");
				fragment.append("hres = S_OK;\n");
				fragment.append("}\n");
			}

			 fragment.append("break;\n");
			 fragment.append("}\n");
		}

		fragment.append("default:\n");
		fragment.append("break;\n");
		fragment.append("}\n");
		fragment.append("if (SUCCEEDED(hres)) {\n");
		fragment.append("if (pVarResult)\n");
		fragment.append("\t*pVarResult = res;\n");
		fragment.append("else\n");
		fragment.append("\tVariantClear(&res);\n");
		fragment.append("}\n");
		fragment.append("else {\n");
		fragment.append("external_log(LIBWINEVBS_LOG_WARN, \"" + idlInterface.getClassName() + "_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d\", dispIdMember, dispIdMember, wFlags, hres);\n");
		fragment.append("}\n");
		fragment.append("return hres;\n");

		buffer.append(indent(fragment.toString(), 1));
		buffer.append("}\n\n");

		return buffer.toString();
	 }

	 private String generateHeaderVariant(int index, Param param, String type, Method method) {
		StringBuffer buffer = new StringBuffer();

		buffer.append("VARIANT var" + index + ";\n");

		if (param.hasDefaultValue()) {
			 if ("int".equals(param.getType())) {
				buffer.append("V_VT(&var" + index + ") = VT_I4;\n");
				buffer.append("V_I4(&var" + index + ") = " + param.getDefaultValue() + ";\n");
			 }
			 else if ("long".equals(param.getType())) {
				buffer.append("V_VT(&var" + index + ") = VT_I4;\n");
				buffer.append("V_I4(&var" + index + ") = " + param.getDefaultValue() + ";\n");
			 }
			 else if ("float".equals(param.getType())) {
				buffer.append("V_VT(&var" + index + ") = VT_R4;\n");
				buffer.append("V_R4(&var" + index + ") = " + param.getDefaultValue() + ";\n");
			 }
			 else if ("VARIANT_BOOL".equals(param.getType())) {
				buffer.append("V_VT(&var" + index + ") = VT_BOOL;\n");
				buffer.append("V_BOOL(&var" + index + ") = " + param.getDefaultValue() + ";\n");
			 }
			 else if ("BSTR".equals(param.getType())) {
				buffer.append("V_VT(&var" + index + ") = VT_BSTR;\n");
				buffer.append("V_BSTR(&var" + index + ") = SysAllocString(L" + param.getDefaultValue() + ");\n");
			 }
			 else if ("CompareMethod".equals(param.getType())
					 || "Tristate".equals(param.getType())
					 || "IOMode".equals(param.getType())
					 || "StandardStreamTypes".equals(param.getType())
					 || "FileAttribute".equals(param.getType())
					 || "SpecialFolderConst".equals(param.getType())
					 || "DriveTypeConst".equals(param.getType())) {
				buffer.append("V_VT(&var" + index + ") = VT_I4;\n");
				buffer.append("V_I4(&var" + index + ") = " + param.getDefaultValue() + ";\n");
			 }
			 else {
				buffer.append("\nUNSUPPORTED DEFAULT\n");
			 }
		}
		else {
			 buffer.append("V_VT(&var" + index + ") = VT_EMPTY;\n");
		}

		if (type != null) {
			 buffer.append("VariantChangeType(&var" + index + ", ");

			 if (param.hasDefaultValue() || param.isOptional()) {
				buffer.append("(index > 0) ? &pDispParams->rgvarg[--index] : &var" + index);
			 }
			 else {
				buffer.append("&pDispParams->rgvarg[--index]");
			 }

			 buffer.append(", 0, " + type + ");\n");
		}
		else {
			 buffer.append("VariantCopyInd(&var" + index + ", &pDispParams->rgvarg[--index]);\n");
		}

		return buffer.toString();
	 }

	 private String generateMethod(Method method, String className) {
		StringBuffer buffer = new StringBuffer();

		buffer.append("hres = " + className + "_" + method.getPrefix() + method.getName() + "(iface");

		int index = 0;

		StringBuffer header = new StringBuffer();

		for (Param param : method.getParamList()) {
			 buffer.append(", ");

			 if (param.getParamType() == ParamType.IN) {
				if ("int".equals(param.getType())) {
					 header.append(generateHeaderVariant(index, param, "VT_I4", method));
					 buffer.append("V_I4(&var" + index + ")");
				}
				else if ("long".equalsIgnoreCase(param.getType())) {
					 header.append(generateHeaderVariant(index, param, "VT_I4", method));
					 buffer.append("V_I4(&var" + index + ")");
				}
				else if ("float".equals(param.getType())) {
					 header.append(generateHeaderVariant(index, param, "VT_R4", method));
					 buffer.append("V_R4(&var" + index + ")");
				}
				else if ("VARIANT_BOOL".equals(param.getType())) {
					 header.append(generateHeaderVariant(index, param, "VT_BOOL", method));
					 buffer.append("V_BOOL(&var" + index + ")");
				}
				else if ("BSTR".equals(param.getType())) {
					 header.append(generateHeaderVariant(index, param, "VT_BSTR", method));
					 buffer.append("V_BSTR(&var" + index + ")");
				}
				else if ("DATE".equals(param.getType())) {
					 header.append(generateHeaderVariant(index, param, "VT_DATE", method));
					 buffer.append("V_DATE(&var" + index + ")");
				}
				else if ("VARIANT*".equals(param.getType())) {
					 header.append(generateHeaderVariant(index, param, null, method));
					 buffer.append("&var" + index );
				}
				else if ("VARIANT".equals(param.getType())) {
					 header.append(generateHeaderVariant(index, param, null, method));
					 buffer.append("var" + index);
				}
				else if ("OLE_COLOR".equals(param.getType())) {
					 header.append(generateHeaderVariant(index, param, "VT_UI4", method));
					 buffer.append("(OLE_COLOR)V_UI4(&var" + index + ")");
				}
				else if ("CompareMethod".equals(param.getType())
						|| "Tristate".equals(param.getType())
						|| "IOMode".equals(param.getType())
						|| "StandardStreamTypes".equals(param.getType())
						|| "FileAttribute".equals(param.getType())
						|| "SpecialFolderConst".equals(param.getType())
						|| "DriveTypeConst".equals(param.getType())) {
					 header.append(generateHeaderVariant(index, param, "VT_I4", method));
					 buffer.append("(" + param.getType() + ")V_I4(&var" + index + ")");
				}
				else if ("IFontDisp*".equals(param.getType())) {
					 // FIX ME
					 buffer.append("(IFontDisp*)pDispParams->rgvarg");
				}
				else if ("void".equals(param.getType())) {
				}
				else {
					 buffer.append(param.getType());
				}
			 }
			 else if (param.getParamType() == ParamType.OUT) {
				if ("VARIANT*".equals(param.getType())) {
						header.append("VARIANT* var" + index + " = &pDispParams->rgvarg[--index];\n");
						buffer.append("var" + index);
				}
				else {
					 buffer.append(param.getType());
				}
			 }
			 else if (param.getParamType() == ParamType.OUT_RETVAL) {
				if ("int*".equals(param.getType())) {
					 buffer.insert(0, "V_VT(&res) = VT_I4;\n");
					 buffer.append("(int*)&V_I4(&res)");
				}
				else if ("float*".equals(param.getType())) {
					 buffer.insert(0, "V_VT(&res) = VT_R4;\n");
					 buffer.append("&V_R4(&res)");
				}
				else if ("long*".equalsIgnoreCase(param.getType())) {
					 buffer.insert(0, "V_VT(&res) = VT_I4;\n");
					 buffer.append("(LONG*)&V_I4(&res)");
				}
				else if ("VARIANT_BOOL*".equals(param.getType())) {
					 buffer.insert(0, "V_VT(&res) = VT_BOOL;\n");
					 buffer.append("&V_BOOL(&res)");
				}
				else if ("BSTR*".equals(param.getType())) {
					 buffer.insert(0, "V_VT(&res) = VT_BSTR;\n");
					 buffer.append("&V_BSTR(&res)");
				}
				else if ("DATE*".equals(param.getType())) {
					 buffer.insert(0, "V_VT(&res) = VT_DATE;\n");
					 buffer.append("&V_DATE(&res)");
				}
				else if ("OLE_COLOR*".equals(param.getType())) {
					 buffer.insert(0, "V_VT(&res) = VT_UI4;\n");
					 buffer.append("&V_UI4(&res)");
				}
				else if ("VARIANT*".equals(param.getType())) {
					 buffer.append("&res");
				}
				else if ("SIZE_T*".equals(param.getType())) {
					 buffer.insert(0, "V_VT(&res) = VT_UI4;\n");
					 buffer.append("(SIZE_T*)&V_UI4(&res)");
				}
				else if ("IUnknown**".equals(param.getType())) {
					 buffer.insert(0, "V_VT(&res) = VT_UNKNOWN;\n");
					 buffer.append("&V_UNKNOWN(&res)");
				}
				else if ("IFontDisp**".equals(param.getType())
						|| "ITable**".equals(param.getType())
						|| "IDrive**".equals(param.getType())
						|| "IDriveCollection**".equals(param.getType())
						|| "IFolderCollection**".equals(param.getType())
						|| "IFileCollection**".equals(param.getType())
						|| "IFile**".equals(param.getType())
						|| "IFolder**".equals(param.getType())
						|| "ITextStream**".equals(param.getType())
						|| "IWshCollection**".equals(param.getType())
						|| "IWshEnvironment**".equals(param.getType())
						|| "IWshExec**".equals(param.getType())
						|| "IDispatch**".equals(param.getType())) {
					 buffer.insert(0, "V_VT(&res) = VT_DISPATCH;\n");
					 buffer.append("(" + param.getType() + ")&V_DISPATCH(&res)");
				}
				else if ("SAFEARRAY(VARIANT)*".equals(param.getType())) {
					 buffer.insert(0, "V_VT(&res) = VT_VARIANT|VT_ARRAY;\n");
					 buffer.append("(SAFEARRAY**)&V_ARRAY(&res)");
				}
				else if ("CompareMethod*".equals(param.getType())
						|| "Tristate*".equals(param.getType())
						|| "IOMode*".equals(param.getType())
						|| "StandardStreamTypes*".equals(param.getType())
						|| "FileAttribute*".equals(param.getType())
						|| "SpecialFolderConst*".equals(param.getType())
						|| "DriveTypeConst*".equals(param.getType())
						|| "WshExecStatus*".equals(param.getType())) {
					 buffer.insert(0, "V_VT(&res) = VT_I4;\n");
					 buffer.append("(" + param.getType() + ")&V_I4(&res)");
				}
				else {
					 buffer.append(param.getType());
				}
			 }

			 index++;
		}

		buffer.append(");\n");

		if (isChainablePropget(method)) {
			 buffer.append("if (SUCCEEDED(hres) && pDispParams->cArgs > 0) {\n");
			 buffer.append("IDispatch *_chained = V_DISPATCH(&res);\n");
			 buffer.append("V_VT(&res) = VT_EMPTY;\n");
			 buffer.append("hres = IDispatch_Invoke(_chained, DISPID_VALUE, &IID_NULL, lcid, wFlags, pDispParams, &res, pExcepInfo, puArgErr);\n");
			 buffer.append("IDispatch_Release(_chained);\n");
			 buffer.append("}\n");
		}

		index = 0;

		for (Param param : method.getParamList()) {
			 if (param.getParamType() != ParamType.OUT &&
				param.getParamType() != ParamType.OUT_RETVAL) {
				buffer.append("VariantClear(&var" + index + ");\n");
			 }
			 index++;
		}

		buffer.insert(0, header.toString());

		if (method.getMinRequiredArgs() > 0) {
			 buffer.insert(0, "if (pDispParams->cArgs < " + method.getMinRequiredArgs() + ") {\nhres = DISP_E_BADPARAMCOUNT;\nbreak;\n}\n");
		}

		buffer.insert(0, "// line " + method.getLineNo() + ": " + method.getLine() + "\n");

		String wrap = "if (wFlags & " + method.getDispatchType() +") {\n";
		wrap += buffer.toString();
		wrap += "}\n";

		return wrap;
	 }

	 private String indent(String code, int indent) throws Exception {
		StringBuffer buffer = new StringBuffer();

		for (int index = 0; index < indent; index++) {
			 buffer.append("\t");
		}

		boolean newLine = false;

		for (int i = 0; i < code.length(); i++) {
			 char character = code.charAt(i);

			 if (character == '\n') {
				newLine = true;
				buffer.append(character);
			 }
			 else {
				if (character == '{') {
					 indent++;
				}
				else if (character == '}') {
					 indent--;
				}

				if (newLine) {
					 newLine = false;
					 for (int index = 0; index < indent; index++) {
						buffer.append("\t");
					 }
				}

				buffer.append(character);
			 }
		}

		return buffer.toString();
	 }

	 public static void main(String[] args) throws Exception {
		IDLParserToC parser = new IDLParserToC();

		String wineDlls = "../../wine/dlls";

		parser.parse(
				wineDlls + "/scrrun/scrrun.idl",
				wineDlls + "/scrrun/dictionary_proxy.c",
				Arrays.asList(
					new IDLInterface("IDictionary", "dictionary")));

		parser.parse(
				wineDlls + "/scrrun/scrrun.idl",
				wineDlls + "/scrrun/filesystem_proxy.c",
				Arrays.asList(
					new IDLInterface("IDriveCollection", "drivecoll"),
					new IDLInterface("IFolderCollection", "foldercoll"),
					new IDLInterface("IFolder", "folder"),
					new IDLInterface("IFileCollection", "filecoll"),
					new IDLInterface("IDrive", "drive"),
					new IDLInterface("ITextStream", "textstream"),
					new IDLInterface("IFile", "file"),
					new IDLInterface("IFileSystem3", "filesys")
					));

		parser.parse(
				wineDlls + "/vbscript/vbsregexp55.idl",
				wineDlls + "/vbscript/regexp_proxy.c",
				Arrays.asList(
					new IDLInterface("IRegExp", "RegExp"),
					new IDLInterface("IRegExp2", "RegExp2"),
					new IDLInterface("IMatch", "Match"),
					new IDLInterface("IMatch2", "Match2"),
					new IDLInterface("IMatchCollection", "MatchCollection"),
					new IDLInterface("IMatchCollection2", "MatchCollection2"),
					new IDLInterface("ISubMatches", "SubMatches")));

		parser.parse(
				wineDlls + "/wshom.ocx/wshom.idl",
				wineDlls + "/wshom.ocx/shell_proxy.c",
				Arrays.asList(
					new IDLInterface("IWshCollection", "WshCollection"),
					new IDLInterface("IWshEnvironment", "WshEnvironment"),
					new IDLInterface("IWshExec", "WshExec"),
					new IDLInterface("IWshShortcut", "WshShortcut"),
					new IDLInterface("IWshShell3", "WshShell3")));
	 }
}
