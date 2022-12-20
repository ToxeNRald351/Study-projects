<?php

define("MANUAL", "\nusage: Interpret.py [--help] [--directory PATH] [--recursive] [--parse-script FILE] [--int-script FILE] [--parse-only] [--int-only] [--jexampath PATH] [--noclean]
optional arguments:
  --help                show this info
  --directory=PATH      foulder to search for test files
  --recursive           search for tests not only in the specified directory, but also recursively in all its subdirectories
  --parse-script=FILE   use this script for parse
  --int-script=FILE     use this script for interpretation
  --parse-only          do parse only
  --int-only            do interpretation only
  --jexampath=PATH      path to jexamxml.jar file
  --noclean             help files with intermediate results will not be deleted\n");

function read_args() {
    #default values
    $def_path = getcwd();
    $args = array(
        "dir_path"              => $def_path,
        "rec"                   => false,
        "parse_script_file"     => "\"" . $def_path . "/parse.php" . "\"",
        "int_script_file"       => "\"" . $def_path . "/interpret.py" . "\"",
        "parse_only"            => false,
        "int_only"              => false,
        "jexam_path"            => "/pub/courses/ipp/jexamxml/",
        "noclean"               => false
    );
    
    $shortopts = "";
    $longopts = array(
        "help", "directory::", "recursive", "parse-script::", "int-script::", "parse-only", "int-only", "jexampath::", "noclean"
    );
    $options = getopt($shortopts, $longopts);

    if ($options) {
        if (array_key_exists("help", $options)) {
            if (count($options) > 1) {
                exit(10);
            }
            fwrite(STDOUT, MANUAL);
            exit(0);
        }

        if (array_key_exists("directory", $options) and $options["directory"] !== false ?? false) {
            if (!file_exists($options["directory"])) {
                exit(41);
            }
            
						$args["dir_path"] = $options["directory"];
        }
        if (array_key_exists("recursive", $options)) {
            $args["rec"] = true;
        }

        if (array_key_exists("parse-script", $options) and $options["parse-script"] !== false ?? false) {
            if (!file_exists($options["parse-script"])) {
                exit(41);
            }
            
            $args["parse_script_file"] = $options["parse-script"];
        }

        if (array_key_exists("int-script", $options) and $options["int-script"] !== false ?? false) {
            if (!file_exists($options["int-script"])) {
                exit(41);
            }
						
						$args["int_script_file"] = $options["int-script"];
        }

        if (array_key_exists("parse-only", $options)) {
            if (array_key_exists("int-only", $options) or array_key_exists("int-script", $options)) {
                exit(10);
            }
            $args["parse_only"] = true;
        }

        if (array_key_exists("int-only", $options)) {
            if (array_key_exists("parse-only", $options) or array_key_exists("parse-script", $options) or array_key_exists("jexampath", $options)) {
                exit(10);
            }
            $args["int_only"] = true;
        }

        if (array_key_exists("jexampath", $options) and $options["jexampath"] !== false ?? false) {
            if (!file_exists($options["jexampath"])) {
                exit(41);
            }
            
            $args["jexam_path"] = $options["jexampath"];
        }

        if (array_key_exists("noclean", $options)) {
            $args["noclean"] = true;
        }
    }
    else if ($options === false) {
        fwrite(STDERR, "ERROR: internal error!\n");
        exit(99);
    }
    return $args;
}

function get_src($string) {
    $elements = explode("\n", $string);
    if ($elements[0][1] != "/") {
        $foulder = substr(array_shift($elements), 0, -1);
    }
    else {
        $foulder = substr(array_shift($elements), 2, -1);
    }
    
    if (!empty($foulder)) {
        if ($foulder == ".") {
            $foulder = "/";
        }
        else {
          $foulder = $foulder . "/";
        }
    }
    
    $filenames = array();
    
    if (empty($elements)) { 
        return; 
    }

    foreach ($elements as $name) {
        if (preg_match('/\.src$/', $name)) {
            $filenames[] = $foulder . $name;
        }
    }

    return $filenames;
}

$args = read_args();
#Get paths
$file_paths = array();
if ($args["rec"]) {
    $content = shell_exec("ls -R " . $args["dir_path"]);
    $content = explode("\n\n", $content);
    if (!empty($content)) {
        foreach ($content as $string) {
            $file_paths = array_merge($file_paths, get_src($string));
        }
    }
}
else {
    $content = shell_exec("ls " . $args["dir_path"]);
    $string = explode("\n\n", $content);
    $file_paths = get_src(".:\n" . $string[0]);
    $full_paths = [];
		foreach ($file_paths as $f) {
        $full_paths[] = $args["dir_path"] . $f;
    }
    $file_paths = $full_paths;
}
#
$ref_array = [];
$err_array = [];
$diff_array = [];
#Parse
if (!$args["parse_only"] and !$args["int_only"]) {
    foreach ($file_paths as $file) {
        ###Files with which we're gonna work with
				$parse_alg_number = 0;
        $parse_res_file = "/var/tmp/" . substr(hash(hash_algos()[$parse_alg_number], $file), 0, 7) . ".prs";
        ###
				$rc_file = substr($file, 0, -3) . "rc";
				###
        $out_file = "\"" . substr($file, 0, -3) . "out" . "\"";
        if (!file_exists($out_file)) {
            shell_exec("touch " . $out_file);
        }
        ###
        $in_file = "\"" . substr($file, 0, -3) . "in" . "\"";
		    if (!file_exists($in_file)) {
		        shell_exec("touch " . $in_file);
		    }
				###
        $int_alg_munber = 0;
        $int_res_file = "/var/tmp/" . substr(hash(hash_algos()[$parse_alg_number], $file), 0, 7) . ".ntp";
				###
				$err_code = 0;
				$ref_code;
        ###
				if (!file_exists($rc_file)) {
				    $handle = fopen($rc_file, "w+");
				    if (!$handle) {
                exit(99);
            }
				    fputs($handle, "0");
				    $ref_code = 0;
				    fclose($handle);
				}
				else {
				    $handle = fopen($rc_file, "r");
				    if (!$handle) {
                exit(99);
            }
            $ref_code = (int)fgets($handle);
				    fclose($handle);
				}
        #
        #Do parse
        #
        #1
        #Create a unique file to write parse result to
        while (file_exists($parse_res_file)) {
            $parse_alg_number += 1;
            $parse_res_file = "/var/tmp/" . substr(hash(hash_algos()[$parse_alg_number], $file), 0, 7) . ".prs";
        }
        $handle = fopen($parse_res_file, "w+");
        if (!$handle) {
            exit(99);
        }
        #Execute parsing
        exec("php8.1 " . $args["parse_script_file"] . " < " . "\"" . $file . "\"", $result, $err_code);
        $result = implode("", $result);
        #Write result to file
        if ($result) {
            fwrite($handle, $result);
        }
        else {
            fwrite($handle, "");
        }
				fclose($handle);
				#Check error code
        if ($err_code) {
            $err_array[] = $err_code;
            $ref_array[] = $ref_code;
            $diff_array[] = "\n";
            continue;
        }
				#Do compare???
				#
				#
				#
        #Parsing done point here
        #Now to do interpretation
        #Create a unique file
        while (file_exists($int_res_file)) {
            $int_alg_munber += 1;
            $int_res_file = "/var/tmp/" . substr(hash(hash_algos()[$int_alg_munber], $file), 0, 7) . ".ntp";
        }
        $handle = fopen($int_res_file, "w+");
        if (!$handle) {
            exit(99);
        }
        #Now do int
        exec("python3 " . $args["int_script_file"] . " --source=" . $parse_res_file . " --input=" . $in_file, $result, $err_code);
        $result = implode("", $result);
        
				if ($result) {
            fwrite($handle, $result);
        }
        else {
            fwrite($handle, "");
        }
        fclose($handle);
				#Check error code
        if ($err_code) {
            $err_array[] = $err_code;
            $ref_array[] = $ref_code;
            $diff_array[] = "\n";
            continue;
        }
        #Int done
        #Do compare
        $result = shell_exec("diff " . $int_res_file . " " . $out_file);
        $err_array[] = $err_code;
        $ref_array[] = $ref_code;
        $diff_array[] = $result . "\n";
        
        if (!$args["noclean"]) {
            shell_exec("rm " . $parse_res_file);
            shell_exec("rm " . $int_res_file);
        }
    }
}
#Parse only
else if ($args["parse_only"]) {
    foreach ($file_paths as $file) {
				###Files with which we're gonna work with
				$parse_alg_number = 0;
        $parse_res_file = "/var/tmp/" . substr(hash(hash_algos()[$parse_alg_number], $file), 0, 7) . ".prs";
        ###
				$rc_file = substr($file, 0, -3) . "rc";
				###
        $out_file = "\"" . substr($file, 0, -3) . "out" . "\"";
        if (!file_exists($out_file)) {
            shell_exec("touch " . $out_file);
        }
				###
				$err_code = 0;
				$ref_code;
        ###
				if (!file_exists($rc_file)) {
				    $handle = fopen($rc_file, "w+");
				    if (!$handle) {
                exit(99);
            }
				    fputs($handle, "0");
				    $ref_code = 0;
				    fclose($handle);
				}
				else {
				    $handle = fopen($rc_file, "r");
				    if (!$handle) {
                exit(99);
            }
            $ref_code = (int)fgets($handle);
				    fclose($handle);
				}
        #
        #Do parse
        #
        #1
        #Create a unique file to write parse result to
        while (file_exists($parse_res_file)) {
            $parse_alg_number += 1;
            $parse_res_file = "/var/tmp/" . substr(hash(hash_algos()[$parse_alg_number], $file), 0, 7) . ".prs";
        }
        $handle = fopen($parse_res_file, "w+");
        if (!$handle) {
            exit(99);
        }
        #Execute parsing
        exec("php8.1 " . $args["parse_script_file"] . " < " . "\"" . $file . "\"", $result, $err_code);
        $result = implode("", $result);
        #Write result to file
        if ($result) {
            fwrite($handle, $result);
        }
        else {
            fwrite($handle, "");
        }
				fclose($handle);
				#Check error code
        if ($err_code) {
            $err_array[] = $err_code;
            $ref_array[] = $ref_code;
            $diff_array[] = "\n";
            continue;
        }
        #Parsing done point here
				#Do compare
				#
				#
				#
        
        $result = shell_exec("java -jar " . $args["jexam_path"] . "jexamxml.jar " . $out_file . " " . $parse_res_file);
        $err_array[] = $err_code;
        $ref_array[] = $ref_code;
        $diff_array[] = $result . "\n";
        
        if (!$args["noclean"]) {
            shell_exec("rm " . $parse_res_file);
        }
    }
}
#Int-only
else if ($args["int_only"]) {
    foreach ($file_paths as $file) {
		    ###Files with which we're gonna work with
				$rc_file = substr($file, 0, -3) . "rc";
				###
        $out_file = "\"" . substr($file, 0, -3) . "out" . "\"";
        if (!file_exists($out_file)) {
            shell_exec("touch " . $out_file);
        }
        ###
        $in_file = "\"" . substr($file, 0, -3) . "in" . "\"";
		    if (!file_exists($in_file)) {
		        shell_exec("touch " . $in_file);
		    }
				###
        $int_alg_munber = 0;
        $int_res_file = "/var/tmp/" . substr(hash(hash_algos()[$int_alg_munber], $file), 0, 7) . ".ntp";
				###
				$err_code = 0;
				$ref_code;
        ###
				if (!file_exists($rc_file)) {
				    $handle = fopen($rc_file, "w+");
				    if (!$handle) {
                exit(99);
            }
				    fputs($handle, "0");
				    $ref_code = 0;
				    fclose($handle);
				}
				else {
				    $handle = fopen($rc_file, "r");
				    if (!$handle) {
                exit(99);
            }
            $ref_code = (int)fgets($handle);
				    fclose($handle);
				}
				#Now to do interpretation
        #Create a unique file
        while (file_exists($int_res_file)) {
            $int_alg_munber += 1;
            $int_res_file = "/var/tmp/" . substr(hash(hash_algos()[$int_alg_munber], $file), 0, 7) . ".ntp";
        }
        $handle = fopen($int_res_file, "w+");
        if (!$handle) {
            exit(99);
        }
        #Now do int
        
        exec("python3 " . $args["int_script_file"] . " --source=" . $file . " --input=" . $in_file, $result, $err_code);
        $result = implode("", $result);
        
        if ($result) {
            fwrite($handle, $result);
        }
        else {
            fwrite($handle, "");
        }
        fclose($handle);
				#Check error code
        $err_array[] = $err_code;
        $ref_array[] = $ref_code;
        if ($err_code) {
            $diff_array[] = "\n";
            continue;
        }
        #Int done
        #Do compare
        $result = shell_exec("diff " . $int_res_file . " " . $out_file);
        $diff_array[] = $result . "\n";
        
        if (!$args["noclean"]) {
            shell_exec("rm " . $int_res_file);
        }
    }
}
#Vypis statistik
$n = 0;
for ($i = 0; $i < count($err_array); $i++) {
    if ($err_array[$i] != $ref_array[$i])
        $n++;
}
$test_fail = $n;
$test_pass = count($err_array) - $n;

echo "
<!DOCTYPE html
<html lang=\"en\">
    <head>
        <meta charset=\"utf-8\">
        <meta name=\"test stat\" content=\"result\">
    </head>
    <body>";
$i = 0;
foreach ($file_paths as $file) {
    echo "
        <h3>file order=\"" . $i + 1 . "\" name=<\"" . $file . "\"</h3>
        <ul>
            <li>stat name=\"reference exit code\"</li>
            <li>value=\"" . $ref_array[$i] . "\"</li>
            <li>stat name=\"test exit code\"</li>
            <li>value=\"" . $err_array[$i] . "\"</li>
        </ul>";
		
		if ($ref_array[$i] == $err_array[$i]) {
        $diff = array_shift($diff_array);
				if ($diff == "\n") { 
            $diff = "No difference";
        }
        elseif (str_contains($diff, "Error(s)! See log file")) {
            $test_pass -= 1;
            $test_fail += 1;
				}
        elseif (str_contains($diff, "Two files are identical")) {
				}
        else {
            $test_pass -= 1;
            $test_fail += 1;
        }
        echo "
        <p>" . $diff . "</p>\n";
    }
    else { array_shift($diff_array); }
    $i += 1;
}
echo "
        <h3>total_tests=\"" . count($err_array) . "\"</h3>\n";
echo "
        <h3>total tests_failed=\"" . $test_fail . "\" tests_passed=\"" . $test_pass . "\"</h3>
    </body>
</html>\n";

exit(0);
?>