'use strict';

const sourcePath = /^src\/(?:bytecode|compiler|packages|project_model|stdlib)\/.+\.panack$/;

function validateManifest(manifest, tracked) {
  if (manifest?.schema !== 1 || !Array.isArray(manifest.eligible) ||
      !Array.isArray(manifest.unmeasured) || !Array.isArray(tracked)) {
    throw new Error('Invalid source coverage manifest');
  }
  const eligible = manifest.eligible;
  const unmeasured = manifest.unmeasured.map(entry => entry?.path);
  if (eligible.some(path => typeof path !== 'string' || !sourcePath.test(path)) ||
      unmeasured.some((path, index) => typeof path !== 'string' || !sourcePath.test(path) ||
        typeof manifest.unmeasured[index]?.reason !== 'string' || !manifest.unmeasured[index].reason.trim())) {
    throw new Error('Invalid measured or unmeasured source entry');
  }
  const accounted = [...eligible, ...unmeasured];
  if (new Set(eligible).size !== eligible.length || new Set(unmeasured).size !== unmeasured.length ||
      accounted.some((path, index) => accounted.indexOf(path) !== index)) {
    throw new Error('Duplicate or overlapping source coverage entries');
  }
  const actual = [...new Set(tracked)].sort();
  if (actual.length !== tracked.length || tracked.some(path => !sourcePath.test(path)) ||
      JSON.stringify([...accounted].sort()) !== JSON.stringify(actual)) {
    throw new Error('Measured and unmeasured manifest entries differ from tracked src/**/*.panack');
  }
  return {eligible: [...eligible], unmeasured: manifest.unmeasured.map(({path, reason}) => ({path, reason}))};
}

module.exports = {validateManifest};
