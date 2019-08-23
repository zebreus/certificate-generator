#!/usr/local/bin/thrift --gen cpp

namespace cpp CertificateGeneratorThrift

exception InvalidConfiguration {
1: string message,
}

exception InvalidTemplate {
1: string message,
}

exception UnableToWriteFile {
1: string message,
}

service CertificateGenerator {
  string generateCertificate(1:string configuration ),
}
