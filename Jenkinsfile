#!groovy

properties([disableConcurrentBuilds(), pipelineTriggers([pollSCM('H/3 * * * *')])])

pipeline {
	agent { label 'ubuntu-llvm' }
	environment {
		REPO_NAME = "github.com/ShiftLeftSecurity/llvm2cpg"
		CODEPROPERTYGRAPH_REPO_NAME = "github.com/ShiftLeftSecurity/codepropertygraph"
		GITHUB_KEY = "4b3482c3-735f-4c31-8d1b-d8d3bd889348"
	}
	options {
		skipDefaultCheckout()
	}
	stages {
		stage('cleanUp') {
			steps {
				script {
					try {
						deleteDir()
					} catch (err) {
						println("WARNING: Failed to delete directory: " + err)
					}
				}
			}
		}
		stage('getSrc') {
			steps {
				script {
					sshagent(credentials: ["${env.GITHUB_KEY}"]) {
						checkout([
								$class                           : 'GitSCM',
								branches                         : [[name: "*/master"]],
								doGenerateSubmoduleConfigurations: false,
								extensions                       : [[
												   $class             : 'SubmoduleOption',
												   disableSubmodules  : false,
												   parentCredentials  : false,
												   recursiveSubmodules: true,
												   reference          : '',
												   trackingSubmodules : false
												   ]],
								submoduleCfg                     : [],
								userRemoteConfigs                : [[
												   url: "ssh://git@${env.REPO_NAME}"
												   ]]
						])
					}
					sshagent(credentials: ["${env.GITHUB_KEY}"]) {
						sh "git clone ssh://git@${env.CODEPROPERTYGRAPH_REPO_NAME}"
					}
				}
			}
		}
		stage('prepare') {
			steps {
				script {
					dir('target') {
						sh "cmake -G Ninja \
						    -DCMAKE_C_COMPILER=/opt/llvm/9.0.0/bin/clang \
						    -DCMAKE_CXX_COMPILER=/opt/llvm/9.0.0/bin/clang++ \
						    -DPATH_TO_LLVM=/opt/llvm/9.0.0/ \
						    -DPATH_TO_CODEPROPERTYGRAPH=${WORKSPACE}/codepropertygraph \
						    .."
					}
				}
			}
		}
		stage('run-all-tests') {
			steps {
				script {
					dir('target') {
						sh "ninja run-all-tests"
					}
				}
			}
		}
	}
	post {
		failure {
			script {
				notifyFailed()
			}
		}
		fixed {
			script {
				notifyFixed()
			}
		}
	}
}

def notifyFailed() {
	slackSend (channel: '#dev-null', color: '#FF0000', message: "FAILED: Job '${env.JOB_NAME} [${env.BUILD_NUMBER}]' (${env.BUILD_URL})")
	//emailext body: "Build URL: ${env.BUILD_URL} (to view full results, click on \"Console Output\")", attachLog: true, recipientProviders: [[$class: 'CulpritsRecipientProvider']], subject: 'Action Required: Jenkins $JOB_NAME #$BUILD_NUMBER FAILED', to: 'build-notify-code-science@shiftleft.io'
}

def notifyFixed() {
	slackSend (channel: '#team-llvm', color: '#22FF00', message: "FIXED: Job '${env.JOB_NAME} [${env.BUILD_NUMBER}]' (${env.BUILD_URL})")
	//emailext body: "Build URL: ${env.BUILD_URL} (to view full results, click on \"Console Output\")", attachLog: true, recipientProviders: [[$class: 'CulpritsRecipientProvider']], subject: 'Notice: Jenkins $JOB_NAME #$BUILD_NUMBER FIXED!', to: 'build-notify-code-science@shiftleft.io'
}
