#!/usr/bin/python

import sys
import os
import getopt
import re
import subprocess

def usage():
    sys.stdout.write(''' Usage: %s [-hH?]
    ''')
    sys.exit(99)

def unused(args):
    #remove warning
    pass

def parseArgs():
    src = chk = bld = None;

    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hH?cso:', ['check', 'start', 'help', 'source='])
    except getopt.GetoptError as err:
        print str(err)
        usage()
    unused(args)

    for o, a in opts:
        if o in ['-h', '-H', '-?', '--help']:
            usage()
        elif o in ['-c', '--check']:
            chk = True
        elif o in ['-s', '--start']:
            bld = True
        elif o in ['-o', '--source']:
            src = a
        else:
			pass
    return (chk, bld, src);

def cur_file_dir():
    # get current path
    path = sys.path[0]
    if os.path.isdir(path):
        return path
    elif os.path.isfile(path):
        return os.path.dirname(path)

def writeSpaces(fp, num):
    for i in range(num):
        fp.write(' ')

def checkBuildEnvironment():

    # check python version
    pythonCmd = os.getenv('PYTHON_CMD')
    try:
        pythonVersion = subprocess.Popen(pythonCmd + ' --version', stderr=subprocess.PIPE).stderr.read()
    except OSError:
        print('* Error occurs: Python command is not found')
        sys.exit(99)
    antPattern = re.compile(r'(\d+\.){2}\d+')
    match = antPattern.search(pythonVersion)
    if match:
        print 'Using Python version: %s' % match.group(0)

    # check ant version
    antCmd = os.getenv('ANT_CMD')
    try:
        antVersion = subprocess.Popen(antCmd + ' -version', stdout=subprocess.PIPE).stdout.read()
    except OSError:
        print('* Error occurs: Ant command is not found')
        sys.exit(99)
    antPattern = re.compile(r'(\d+\.){2}\d+')
    match = antPattern.search(antVersion)
    if match:
        print 'Using Ant version: %s' % match.group(0)

    # check javac version
    try:
        javacVersion = subprocess.Popen('javac -version', stderr=subprocess.PIPE).stderr.read()
    except OSError:
        print('* Error occurs: javac command is not found')
        sys.exit(99)

    javacPattern = re.compile(r'(\d+\.){2}\d+(_\d{2})?')
    match = javacPattern.search(javacVersion)
    if match:
        print 'Using javac version: %s' % match.group(0)
    
    # check dx tool version
    dxCmd = os.getenv('DX_TOOL')
    try:
        dxVersion = subprocess.Popen(dxCmd + ' --version', stderr=subprocess.PIPE).stderr.read()
    except OSError:
        print('* Error occurs: dx command is not found')
        sys.exit(99)
    antPattern = re.compile(r'(\d+\.){1,2}\d+')
    match = antPattern.search(dxVersion)
    if match:
        print 'Using dx tool version: %s' % match.group(0)

    print 'All tools are installed, please run build with command "build.bat start"'

def generateAntFile(src_path):
    print '# Generating Ant build files:'
    
    indents = 0

    rootPath = os.path.abspath(cur_file_dir()+'/..') 
    #print "rootPath = %s" % rootPath
    if not os.path.exists(rootPath + '/gen'):
        os.mkdir(rootPath + '/gen')
    antFile = rootPath + '/gen/tmp_build.xml'
    antFp = open(antFile, 'w')
    
    #write header
    antFp.write('<?xml version="1.0" encoding="utf-8"?>\r\n')
    #write comments
    antFp.write('''
<!--
@Copyright [2013] - [2015] Yarlungsoft.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

NOTE:
    This file is generate automatically, please don't modify it manually.
-->\r\n''')

    #define properties
    properties = {
        "src.dir" : src_path,
        "classes.dir": "bin",
        "core-lib.dir": "core-lib",
        "output.dir": "gen"
    }

    #write project and properties
    antFp.write('<project default="java_build" basedir="..">\r\n')
    indents = 4
    for key in properties:
        writeSpaces(antFp, indents)
        antFp.write('<property name = ' + '"' + key + '"' + ' ' + 'value = ' + '"'
        + properties[key] + '"' + '></property>\r\n')

    # line separate
    antFp.write("\r\n");
    
    #internal class path
    writeSpaces(antFp, indents)
    antFp.write('<!-- classpath -->\r\n')
    writeSpaces(antFp, indents)
    antFp.write('''<path id="lib.jar.classpath">  
        <fileset file="${core-lib.dir}/*.jar"></fileset>  
        <pathelement location="${classes.dir}"></pathelement>  
    </path>\r\n\r\n''')
    
    #create Folders
    writeSpaces(antFp, indents)
    antFp.write('<!-- create folders -->')
    antFp.write('''
    <target name="createOutputFolder">
        <mkdir dir="${output.dir}"/>
        <mkdir dir="${output.dir}/${classes.dir}"/>
    </target>\r\n\r\n''')

    #target build
    writeSpaces(antFp, indents)
    antFp.write('<!-- target build -->')
    antFp.write('''
    <target name="compile" depends="createOutputFolder">  
        <javac srcdir="${src.dir}" encoding="utf-8" debug="true"   
            destdir="${output.dir}/${classes.dir}"  includeantruntime="true" >  
            <classpath refid="lib.jar.classpath"></classpath>  
        </javac>
    </target>\r\n\r\n''')

    #package classes
    writeSpaces(antFp, indents)
    antFp.write('<!-- package classes -->')
    antFp.write('''
    <target name="dist" depends="compile">
        <tstamp prefix="buildtime" >    
            <format property="TimeSign" pattern="yyyyMMddHHmmss" />    
        </tstamp> 
        <jar destfile="${output.dir}/package-${buildtime.TimeSign}.jar" basedir="${output.dir}/${classes.dir}">
            <manifest>
                <attribute name="Built-By" value="Yarlungsoft"/>
                <attribute name="Built-Date" value="${buildtime.TimeSign}"/>
            </manifest>
        </jar>
    </target>\r\n''')

    antFp.write('</project>\r\n')
    antFp.close()

    return antFile

def endWith(s, *endstring):
    array = map(s.endswith,endstring)
    if True in array:
        return True
    else:
        return False

def findJarFile(path):
    s = os.listdir(path)
    files = []
    for i in s:
        if endWith(i, '.jar'):
            files.append(i)
       
    st = files.sort(key=lambda fn: os.path.getmtime(path+'\\'+fn) \
        if not os.path.isdir(path+'\\'+fn) else 0)
    return files[-1]

def buildAndGenerateDex(antFile):

    antCmd = os.getenv('ANT_CMD') + ' -d' + ' -buildfile ' + '"' + antFile + '"' + ' dist'
    os.system(antCmd)

    genPath = os.path.dirname(antFile)
    jarFileBaseName = findJarFile(genPath)

    dxTool = os.getenv('DX_TOOL')
    dxJarFile  = genPath + '/dex_' + jarFileBaseName
    clsJarFile = genPath + '/' + jarFileBaseName

    dxCmd = dxTool + ' --verbose --dex --output=' + '"' + \
        dxJarFile + '"' + ' ' + '"' + clsJarFile + '"'
    os.system(dxCmd);

    print "#########################################################"
    print "#"
    print "# Class Jar Package: %s" % clsJarFile
    print "# Dex Jar Pakcage: %s" % dxJarFile
    print "#"
    print "#########################################################"

def main():
    res = parseArgs()
    print res
    if (res[0]):
        checkBuildEnvironment()

    if (res[1]):
        antFile = generateAntFile(res[2])
        buildAndGenerateDex(antFile)


if __name__ == '__main__':
    main()


