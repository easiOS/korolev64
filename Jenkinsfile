node {
	stage('Checkout') {
		checkout scm
	}
	stage('Configure') {
		sh 'mkdir -p build'
		dir('build/') {
			sh 'cmake ../'
		}
	}
	stage('Build') {
		dir('build/') {
			sh 'make -j $(nproc)'
		}
	}
}
